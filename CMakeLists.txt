cmake_minimum_required (VERSION 3.9.0)
project (bsfExamples)

set(BSF_DIRECTORY "${PROJECT_SOURCE_DIR}/../bsf" CACHE STRING "bsf directory")

set (BSF_SOURCE_DIR ${BSF_DIRECTORY}/Source)
set (APP_ROOT_DIR ${BSF_DIRECTORY})
set (SECONDARY_APP_ROOT_DIR "${PROJECT_SOURCE_DIR}")
set (BS_IS_BANSHEE3D 0)

include(${BSF_SOURCE_DIR}/CMake/Properties.cmake)
include(${BSF_SOURCE_DIR}/CMake/FindPackageOrBuild.cmake)
include(${BSF_SOURCE_DIR}/CMake/HelperMethods.cmake)

add_subdirectory(${BSF_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/bsf)

set(BS_EXAMPLES_BUILTIN_ASSETS_VERSION 4)

# Generate config file
configure_file("${PROJECT_SOURCE_DIR}/Source/CMake/BsExampleConfig.h.in" "${PROJECT_SOURCE_DIR}/Source/Common/BsExampleConfig.h")

# Check data dependencies
check_and_update_builtin_assets(bsfExamples ${PROJECT_SOURCE_DIR}/Data ${BS_EXAMPLES_BUILTIN_ASSETS_VERSION} NO)
	
# Sub-directories
add_subdirectory(Source/Common)
add_subdirectory(Source/LowLevelRendering)
add_subdirectory(Source/PhysicallyBasedShading)
add_subdirectory(Source/CustomMaterials)
add_subdirectory(Source/GUI)
add_subdirectory(Source/Audio)
add_subdirectory(Source/SkeletalAnimation)