# Compile steps

- Install git (https://git-scm.com) and CMake 3.9.0 or higher (https://cmake.org)
	- Ensure they are added to your *PATH* environment variable
- Grab, compile and install [bsf](https://github.com/GameFoundry/bsf/blob/master/Documentation/GitHub/compiling.md)
- Run the following commands in the terminal/command line:
	- `git clone https://github.com/GameFoundry/bsfExamples.git`
	- `cd bsfExamples`
	- `mkdir Build`
	- `cd Build`
	- `cmake -G "$generator$" -Dbsf_INSTALL_DIR="path/to/bsf/install" ../`
		- Where *$generator$* should be replaced with any of the supported generators. Some common ones:
			- `Visual Studio 14 2015 Win64` - Visual Studio 2015 (64-bit build)
			- `Visual Studio 15 2017 Win64` - Visual Studio 2017 (64-bit build)
			- `Unix Makefiles`
			- `Ninja`
			- `Xcode`
			- See all valid generators: [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
		- Where `path/to/bsf/install` is the folder where you installed bsf
			- This is the the folder containing `bin`, `lib` and `include` folders of bsf.
			- If installed globally (e.g. `/usr/local` on a Unix system) you do not need to provide the path explicitly)	
- Build the project using your chosen tool
	- Build files are in the `bsfExamples\Build` folder
- Run the examples
	- Example binaries are placed in the `bsfExamples/bin` folder

# Examples
* ExampleGettingStarted - Example covering basic operations, including start up, asset import, camera setup, rendering and GUI.
* ExamplePhysicallyBasedRendering - Example demonstrating the physically based renderer.
* ExampleLowLevelRendering - Example demonstrating how to use the low-level rendering system to manually issue rendering commands. This is similar to using DirectX/OpenGL/Vulkan, except it uses bs::framework's platform-agnostic rendering layer.