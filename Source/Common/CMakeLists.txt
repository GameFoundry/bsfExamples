# Source files and their filters
include(CMakeSources.cmake)

# Target
add_library(Common STATIC ${BS_COMMON_SRC})

# Includes
target_include_directories(Common PUBLIC "./")
	
# Libraries
## Local libs
target_link_libraries(Common bsf)

# IDE specific
set_property(TARGET Common PROPERTY FOLDER Examples)

# Copy bsf binaries
install_dll_on_build(Common ${BSF_DIRECTORY})

# Precompiled header & Unity build
conditional_cotire(Common)