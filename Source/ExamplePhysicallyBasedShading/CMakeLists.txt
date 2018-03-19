# Target
if(WIN32)
	add_executable(ExamplePhysicallyBasedShading WIN32 "Main.cpp")
else()
	add_executable(ExamplePhysicallyBasedShading "Main.cpp")
endif()
	
# Working directory
set_target_properties(ExamplePhysicallyBasedShading PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")		
	
# Libraries
## Local libs
target_link_libraries(ExamplePhysicallyBasedShading Common)