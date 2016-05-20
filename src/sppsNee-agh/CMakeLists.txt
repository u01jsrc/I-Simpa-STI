include_directories (./ "${PROJECT_SOURCE_DIR}/src/lib_interface")

message(STATUS " Boost path - $ENV{BOOST_INCLUDEDIR}")
message(STATUS " Boost lib path - $ENV{BOOST_LIBRARYDIR}")
set(SPPS_SOURCE_FILES
    data_manager/core_configuration.cpp
    input_output/reportmanager.cpp
    tools/dotdistribution.cpp
    CalculationCore.cpp
    sppsNeeAGHInitialisation.cpp
    sppsNeeAGH.cpp
    sppsNeeAGHTypes.cpp)

set(Boost_USE_STATIC_LIBS   ON) 
find_package(Boost REQUIRED COMPONENTS thread random system)
link_directories( ${Boost_LIBRARY_DIRS} )  
include_directories( ${Boost_INCLUDE_DIRS} )

# Set Properties->General->Configuration Type to Application(.exe)
# Creates spps.exe with the listed sources
# Adds sources to the Solution Explorer
add_executable(sppsNee-agh ${SPPS_SOURCE_FILES})

# Properties->Linker->Input->Additional Dependencies
target_link_libraries (sppsNee-agh lib_interface)
target_link_libraries(sppsNee-agh ${Boost_LIBRARIES})

# Creates a folder "executables" and adds target
# project (sppsNee-agh.vcproj) under it
set_property(TARGET sppsNee-agh PROPERTY FOLDER "executables")

# Adds logic to INSTALL.vcproj to copy spps.exe to destination directory
install (TARGETS sppsNee-agh
	CONFIGURATIONS Debug
        RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin/Debug/core/sppsNee-agh)

install (TARGETS sppsNee-agh
	CONFIGURATIONS Release
        RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin/Release/core/sppsNee-agh)