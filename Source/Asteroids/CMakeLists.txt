file(GLOB_RECURSE SRCS
  *.cpp
  *.c
)

# Target
if(WIN32)
  add_executable(Asteroids WIN32 "${SRCS}")
else()
  add_executable(Asteroids "${SRCS}")
endif()

# Working directory
set_target_properties(Asteroids PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

# Libraries
## Local libs
target_link_libraries(Asteroids Common)

# Plugin dependencies
add_engine_dependencies(Asteroids)
add_dependencies(Asteroids bsfFBXImporter bsfFontImporter bsfFreeImgImporter)

# IDE specific
set_property(TARGET Asteroids PROPERTY FOLDER Examples)
