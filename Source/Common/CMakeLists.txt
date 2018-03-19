# Source files and their filters
include(CMakeSources.cmake)

# Target
add_library(Common STATIC ${BS_COMMON_SRC})

# Includes
target_include_directories(Common PUBLIC "./")
	
# Libraries
## Local libs
target_link_libraries(Common ${bsf_LIBRARIES})

# Copy bsf binaries
set(BIN_SRC_DIR "${bsf_INCLUDE_DIRS}/../bin")
set(BIN_DST_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>)

if(WIN32 AND bsf_FOUND)
	file(GLOB BIN_FILES "${BIN_SRC_DIR}/*.dll")
	
	foreach(CUR_PATH ${BIN_FILES})
		get_filename_component(FILENAME ${CUR_PATH} NAME)
	
	   set(SRC ${CUR_PATH})
	   set(DST ${BIN_DST_DIR}/${FILENAME})
	   add_custom_command(
		   TARGET Common POST_BUILD
		   COMMAND ${CMAKE_COMMAND}
		   ARGS    -E copy_if_different ${SRC} ${DST}
		   COMMENT "Copying ${SRC} ${DST}\n"
		   )
	endforeach()
endif()

# Copy bsf data files
set(DATA_SRC_DIR "${BIN_SRC_DIR}/Data")
set(DATA_DST_DIR "${BIN_DST_DIR}/Data")

add_custom_command(
   TARGET Common POST_BUILD
   COMMAND ${CMAKE_COMMAND}
   ARGS    -E copy_directory ${DATA_SRC_DIR} ${DATA_DST_DIR}
   COMMENT "Copying ${DATA_SRC_DIR} ${DATA_DST_DIR}\n"
)