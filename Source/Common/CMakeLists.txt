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
if(WIN32)
	set(BIN_SRC_DIR "${BSF_DIRECTORY}/bin")
	set(BIN_DST_DIR ${PROJECT_SOURCE_DIR}/bin)
	
	file(GLOB_RECURSE BIN_FILES RELATIVE ${BIN_SRC_DIR} "${BIN_SRC_DIR}/*.dll")

	foreach(CUR_PATH ${BIN_FILES})
		get_filename_component(FILENAME ${CUR_PATH} NAME)
	
		if(FILENAME MATCHES "^bsf.*")
			continue()
		endif()
	
		set(SRC ${BIN_SRC_DIR}/${CUR_PATH})
		set(DST ${BIN_DST_DIR}/${CUR_PATH})
		add_custom_command(
		   TARGET Common POST_BUILD
		   COMMAND ${CMAKE_COMMAND}
		   ARGS    -E copy_if_different ${SRC} ${DST}
		   COMMENT "Copying ${SRC} ${DST}\n"
		   )
	endforeach()
endif()