# Target
if(WIN32)
	add_executable(ExampleLowLevelRendering WIN32 "Main.cpp")
else()
	add_executable(ExampleLowLevelRendering "Main.cpp")
endif()
	
# Working directory
set_target_properties(ExampleLowLevelRendering PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")	
	
# Libraries
## Local libs
target_link_libraries(ExampleLowLevelRendering Common)

# Plugin dependencies
add_engine_dependencies(ExampleLowLevelRendering)
add_dependencies(ExampleLowLevelRendering bsfFBXImporter bsfFontImporter bsfFreeImgImporter)

# IDE specific
set_property(TARGET ExampleLowLevelRendering PROPERTY FOLDER Examples)