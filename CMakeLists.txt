cmake_minimum_required (VERSION 3.12.0)
project (bsfExamples)

set (SECONDARY_APP_ROOT_DIR "${PROJECT_SOURCE_DIR}")
set(BS_EXAMPLES_BUILTIN_ASSETS_VERSION 9)

# Generate config file
configure_file("${PROJECT_SOURCE_DIR}/Source/CMake/BsExampleConfig.h.in" "${PROJECT_SOURCE_DIR}/Source/Common/BsExampleConfig.h")

# Check data dependencies
check_and_update_builtin_assets(bsfExamples ${PROJECT_SOURCE_DIR}/Data Data ${BS_EXAMPLES_BUILTIN_ASSETS_VERSION} NO)
	
# Sub-directories
add_subdirectory(Source/Common)
add_subdirectory(Source/LowLevelRendering)
add_subdirectory(Source/PhysicallyBasedShading)
add_subdirectory(Source/CustomMaterials)
add_subdirectory(Source/GUI)
add_subdirectory(Source/Audio)
add_subdirectory(Source/SkeletalAnimation)
add_subdirectory(Source/Physics)
add_subdirectory(Source/Particles)
add_subdirectory(Source/Decals)
add_subdirectory_optional(Source/Experimental/Shadows)
add_subdirectory_optional(Source/Experimental/Particles)