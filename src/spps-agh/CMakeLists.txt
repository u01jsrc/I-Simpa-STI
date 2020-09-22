include_directories (./ "${PROJECT_SOURCE_DIR}/src/lib_interface")
include_directories (./ "${PROJECT_SOURCE_DIR}/src/spps")
set(CMAKE_INSTALL_MFC_LIBRARIES ON)
include( InstallRequiredSystemLibraries )
message(STATUS " Boost path - $ENV{BOOST_INCLUDEDIR}")
message(STATUS " Boost lib path - $ENV{BOOST_LIBRARYDIR}")

#--------------#
#    SOURCES
#--------------#

# Sources files
set(DATA_MANAGER_SOURCES
    data_manager/Core_ConfigurationAGH.h
    data_manager/Core_ConfigurationAGH.cpp)

set(IO_SOURCES
    input_output/reportmanagerAGH.h
    input_output/reportmanagerAGH.cpp)

set(TOOLS_SOURCES
    tools/dotreflectionAGH.h
    tools/brdfreflection.cpp
    tools/brdfreflection.h)

set(SPPS_SOURCES
    sppsNeeAGHTypes.h
    sppsNeeAGHTypes.cpp)
	
set(CALCULATION_CORES 
	CalculationCoreAGH.cpp
	CalculationCoreAGH.h
	calculation_cores/NextEventEstimationCore.cpp 
	calculation_cores/NextEventEstimationCore.h 
	calculation_cores/MLTCore.cpp 
	calculation_cores/MLTCore.h)

set(SPPS_MAIN_SOURCES
	    sppsNeeAGH.cpp)

# Sources groups (folders)
source_group("data_manager" FILES ${DATA_MANAGER_SOURCES})
source_group("input_output" FILES ${IO_SOURCES})
source_group("tools" FILES ${TOOLS_SOURCES})
source_group("calculation_cores" FILES ${CALCULATION_CORES})
source_group("" FILES ${SPPS_SOURCES} ${SPPS_MAIN_SOURCES})

#---------------------------------------#
#    DEPENDENCY & EXECUTABLE (OR LIB)
#---------------------------------------#

set(Boost_USE_STATIC_LIBS   OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_package(Boost REQUIRED COMPONENTS system random filesystem regex)
else()
  find_package (Threads)
  find_package(Boost REQUIRED COMPONENTS system random filesystem regex thread chrono date_time)
endif()

link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

# Check for clock_gettime function
if (UNIX)
  include(CheckLibraryExists)
  check_library_exists(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME )
  if(NOT HAVE_CLOCK_GETTIME)
    message(FATAL_ERROR "clock_gettime not found")
  endif(NOT HAVE_CLOCK_GETTIME)
endif(UNIX)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates sppsNee-agh.exe with the listed sources
# Adds sources to the Solution Explorer
add_executable(spps-agh
    ${DATA_MANAGER_SOURCES}
    ${IO_SOURCES}
    ${TOOLS_SOURCES}
	${CALCULATION_CORES}
    ${SPPS_SOURCES}
	${SPPS_MAIN_SOURCES})

# Creates a folder "executables" and adds target project (spps.vcproj) under it
set_property(TARGET spps-agh PROPERTY FOLDER "executables")

# Add optimization flags
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED)
if(IPO_SUPPORTED)
	set_property(TARGET spps-agh PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()


# Properties->Linker->Input->Additional Dependencies
target_link_libraries (spps-agh spps-lib lib_interface ${Boost_LIBRARIES})


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
  message(STATUS " SPPS debug build disable multi threading")
else()
  message(STATUS " SPPS release build enable multi threading")
  target_link_libraries (spps-agh ${CMAKE_THREAD_LIBS_INIT})
  if(UNIX)
    target_link_libraries (spps-agh rt)
  endif(UNIX)
endif()

#--------------#
#    INSTALL
#--------------#

# Adds logic to INSTALL.vcproj to copy spps.exe to destination directory
install (TARGETS spps-agh
        RUNTIME DESTINATION core/spps-agh)

		
if(WIN32)
	# Resolve symbolic link
	LIB_TO_DLL(${Boost_FILESYSTEM_LIBRARY_RELEASE} Boost_FILESYSTEM_LIBRARY_RELEASE_REAL)
	LIB_TO_DLL(${Boost_SYSTEM_LIBRARY_RELEASE} Boost_SYSTEM_LIBRARY_RELEASE_REAL)
	LIB_TO_DLL(${Boost_RANDOM_LIBRARY_RELEASE} Boost_RANDOM_LIBRARY_RELEASE_REAL)
	LIB_TO_DLL(${Boost_REGEX_LIBRARY_RELEASE} Boost_REGEX_LIBRARY_RELEASE_REAL)
	LIB_TO_DLL(${Boost_THREAD_LIBRARY_RELEASE} Boost_THREAD_LIBRARY_RELEASE_REAL)
	LIB_TO_DLL(${Boost_CHRONO_LIBRARY_RELEASE} Boost_CHRONO_LIBRARY_RELEASE_REAL)
	LIB_TO_DLL(${Boost_DATE_TIME_LIBRARY_RELEASE} Boost_DATE_TIME_LIBRARY_RELEASE_REAL)
	install (FILES
        ${Boost_FILESYSTEM_LIBRARY_RELEASE_REAL}
        ${Boost_SYSTEM_LIBRARY_RELEASE_REAL}
        ${Boost_REGEX_LIBRARY_RELEASE_REAL}
        ${Boost_RANDOM_LIBRARY_RELEASE_REAL}
        ${Boost_THREAD_LIBRARY_RELEASE_REAL}
        ${Boost_CHRONO_LIBRARY_RELEASE_REAL}
        ${Boost_DATE_TIME_LIBRARY_RELEASE_REAL}
			DESTINATION core/spps-agh)
endif()

if( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS )
    install( PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION core/spps-agh	COMPONENT System )
endif( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS )

#------------#
#    TEST
#------------#
# Data files
set(TEST_DATA_DIR "${PROJECT_BINARY_DIR}/Testing")
set(SPPS_TEST_DIR "${TEST_DATA_DIR}/")
#configure_file(tests/config.xml ${SPPS_TEST_DIR}/config.xml)
#file(COPY
#    tests/speaker-test3.txt
#    DESTINATION ${TEST_DATA_DIR})
	

# Test executable and folder
set(SPPS_AGH_TESTS
    tests/spps_agh_core_test.cpp)
	
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost COMPONENTS filesystem regex unit_test_framework timer chrono REQUIRED)

add_executable(TEST_spps_agh ${SPPS_AGH_TESTS}
    ${DATA_MANAGER_SOURCES}
    ${IO_SOURCES}
    ${TOOLS_SOURCES}
    ${SPPS_SOURCES}
	${CALCULATION_CORES})

target_link_libraries (TEST_spps_agh
    lib_interface
	spps-lib
    ${Boost_LIBRARIES})
set_property(TARGET TEST_spps_agh PROPERTY FOLDER "tests")

# Add to test suite
enable_testing()

add_test( NAME spps_agh_core_test
    WORKING_DIRECTORY ${TEST_DATA_DIR}
    COMMAND TEST_spps_agh )
