cmake_minimum_required (VERSION 3.9.0)
project (bsfExamples)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/Source/CMake/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(BS_EXAMPLES_BUILTIN_ASSETS_VERSION 1)

# Configuration types
if(CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE INTERNAL "") 
else()
	if(NOT CMAKE_BUILD_TYPE)
		message("Defaulting to release build.")
		set(CMAKE_BUILD_TYPE Release CACHE INTERNAL "")
	endif()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
	# Set the valid options for cmake-gui drop-down list
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif()

## Generate config file
configure_file("${PROJECT_SOURCE_DIR}/Source/CMake/BsExampleConfig.h.in" "${PROJECT_SOURCE_DIR}/Source/Common/BsExampleConfig.h")

find_package(bsf)

add_subdirectory(Source/Common)
add_subdirectory(Source/ExampleLowLevelRendering)
add_subdirectory(Source/ExamplePhysicallyBasedShading)

## Check data dependencies
set(BSF_SRC_DIR "${bsf_INCLUDE_DIRS}/../bin")

include(Source/CMake/HelperMethods.cmake)
check_and_update_builtin_assets(bsfExamples ${PROJECT_SOURCE_DIR}/Data ${BS_EXAMPLES_BUILTIN_ASSETS_VERSION})

