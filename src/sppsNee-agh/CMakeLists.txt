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
    sppsNeeAGH.cpp
    sppsNeeAGHTypes.h
    sppsNeeAGHTypes.cpp)
	
set(CALCULATION_CORES 
	CalculationCoreAGH.cpp
	CalculationCoreAGH.h
	calculation_cores/NextEventEstimationCore.cpp 
	calculation_cores/NextEventEstimationCore.h 
	calculation_cores/MLTCore.cpp 
	calculation_cores/MLTCore.h)

# Sources groups (folders)
source_group("data_manager" FILES ${DATA_MANAGER_SOURCES})
source_group("input_output" FILES ${IO_SOURCES})
source_group("tools" FILES ${TOOLS_SOURCES})
source_group("calculation_cores" FILES ${CALCULATION_CORES})
source_group("" FILES ${SPPS_SOURCES})

#---------------------------------------#
#    DEPENDENCY & EXECUTABLE (OR LIB)
#---------------------------------------#

set(Boost_USE_STATIC_LIBS   OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_package(Boost REQUIRED COMPONENTS system random filesystem)
else()
  find_package (Threads)
  find_package(Boost REQUIRED COMPONENTS system random filesystem thread chrono date_time)
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
add_executable(sppsNee-agh
    ${DATA_MANAGER_SOURCES}
    ${IO_SOURCES}
    ${TOOLS_SOURCES}
	${CALCULATION_CORES}
    ${SPPS_SOURCES})

# Creates a folder "executables" and adds target project (spps.vcproj) under it
set_property(TARGET sppsNee-agh PROPERTY FOLDER "executables")


# Properties->Linker->Input->Additional Dependencies
target_link_libraries (sppsNee-agh spps-lib lib_interface ${Boost_LIBRARIES})


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
  message(STATUS " SPPS debug build disable multi threading")
else()
  message(STATUS " SPPS release build enable multi threading")
  target_link_libraries (sppsNee-agh ${CMAKE_THREAD_LIBS_INIT})
  if(UNIX)
    target_link_libraries (sppsNee-agh rt)
  endif(UNIX)
endif()

#--------------#
#    INSTALL
#--------------#

# Adds logic to INSTALL.vcproj to copy spps.exe to destination directory
install (TARGETS sppsNee-agh
        RUNTIME DESTINATION core/sppsNee-agh)

		
if(WIN32)
	# Resolve symbolic link
	#LIB_TO_DLL(${Boost_FILESYSTEM_LIBRARY_RELEASE} Boost_FILESYSTEM_LIBRARY_RELEASE_REAL)
	#LIB_TO_DLL(${Boost_SYSTEM_LIBRARY_RELEASE} Boost_SYSTEM_LIBRARY_RELEASE_REAL)
	#LIB_TO_DLL(${Boost_RANDOM_LIBRARY_RELEASE} Boost_RANDOM_LIBRARY_RELEASE_REAL)
	#LIB_TO_DLL(${Boost_REGEX_LIBRARY_RELEASE} Boost_REGEX_LIBRARY_RELEASE_REAL)
	#LIB_TO_DLL(${Boost_THREAD_LIBRARY_RELEASE} Boost_THREAD_LIBRARY_RELEASE_REAL)
	#LIB_TO_DLL(${Boost_CHRONO_LIBRARY_RELEASE} Boost_CHRONO_LIBRARY_RELEASE_REAL)
	#LIB_TO_DLL(${Boost_DATE_TIME_LIBRARY_RELEASE} Boost_DATE_TIME_LIBRARY_RELEASE_REAL)
	install (FILES
        ${Boost_FILESYSTEM_LIBRARY_RELEASE_REAL}
        ${Boost_SYSTEM_LIBRARY_RELEASE_REAL}
        ${Boost_REGEX_LIBRARY_RELEASE_REAL}
        ${Boost_RANDOM_LIBRARY_RELEASE_REAL}
        ${Boost_THREAD_LIBRARY_RELEASE_REAL}
        ${Boost_CHRONO_LIBRARY_RELEASE_REAL}
        ${Boost_DATE_TIME_LIBRARY_RELEASE_REAL}
			DESTINATION core/sppsNee-agh)
endif()

if( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS )
    install( PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION core/sppsNee-agh	COMPONENT System )
endif( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS )
#------------#
#    TEST
#------------#

