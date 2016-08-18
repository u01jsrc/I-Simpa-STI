include_directories (./ "${PROJECT_SOURCE_DIR}/src/lib_interface")

message(STATUS " Boost path - $ENV{BOOST_INCLUDEDIR}")
message(STATUS " Boost lib path - $ENV{BOOST_LIBRARYDIR}")

#--------------#
#    SOURCES
#--------------#

# Sources files
set(DATA_MANAGER_SOURCES
    data_manager/core_configuration.h
    data_manager/core_configuration.cpp)

set(IO_SOURCES
    input_output/reportmanager.h
    input_output/reportmanager.cpp)

set(TOOLS_SOURCES
    tools/dotreflection.h
    tools/dotdistribution.h
    tools/dotdistribution.cpp)

set(SPPS_SOURCES
    CalculationCore.h
    CalculationCore.cpp
    sppsAGHInitialisation.h
    sppsAGHInitialisation.cpp
    sppsAGH.cpp
    sppsAGHTypes.h
    sppsAGHTypes.cpp)

# Sources groups (folders)
source_group("data_manager" FILES ${DATA_MANAGER_SOURCES})
source_group("input_output" FILES ${IO_SOURCES})
source_group("tools" FILES ${TOOLS_SOURCES})
source_group("" FILES ${SPPS_SOURCES})

#---------------------------------------#
#    DEPENDENCY & EXECUTABLE (OR LIB)
#---------------------------------------#

set(Boost_USE_STATIC_LIBS   ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  find_package(Boost REQUIRED COMPONENTS system random filesystem)
else()
  find_package (Threads)
  find_package(Boost REQUIRED COMPONENTS system random filesystem thread)
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
# Creates spps.exe with the listed sources
# Adds sources to the Solution Explorer
add_executable(spps-agh
    ${DATA_MANAGER_SOURCES}
    ${IO_SOURCES}
    ${TOOLS_SOURCES}
    ${SPPS_SOURCES})

# Creates a folder "executables" and adds target project (spps.vcproj) under it
set_property(TARGET spps-agh PROPERTY FOLDER "executables")


# Properties->Linker->Input->Additional Dependencies
target_link_libraries (spps-agh lib_interface ${Boost_LIBRARIES})


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
        RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin/core/spps-agh)

#------------#
#    TEST
#------------#