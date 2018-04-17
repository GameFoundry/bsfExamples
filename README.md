# Compile steps

- Install git (https://git-scm.com) and CMake 3.9.0 or higher (https://cmake.org)
	- Ensure they are added to your *PATH* environment variable
- Run the following commands in the terminal/command line:
    - `git clone https://github.com/GameFoundry/bsf.git`
	- `git clone https://github.com/GameFoundry/bsfExamples.git`
	- `cd bsfExamples`
	- `mkdir Build`
	- `cd Build`
	- `cmake -G "$generator$" ../`
		- Where *$generator$* should be replaced with any of the supported generators. Some common ones:
			- `Visual Studio 14 2015 Win64` - Visual Studio 2015 (64-bit build)
			- `Visual Studio 15 2017 Win64` - Visual Studio 2017 (64-bit build)
			- `Unix Makefiles`
			- `Ninja`
			- `Xcode`
			- See all valid generators: [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
- Build the project using your chosen tool
	- Build files are in the `bsfExamples/Build` folder
- Run the examples
	- Example binaries are placed in the `bsfExamples/bin` folder

# Examples
* CustomMaterials - Demonstrates how to use custom materials that override vertex, surface and lighting aspects of the renderer.
* GUI - Demonstrates how to use the built-in GUI system. Demoes a variety of basic controls, the layout system and shows how to use styles to customize the look of GUI elements.
* PhysicallyBasedRendering - Demonstrates the physically based renderer using the built-in shaders & lighting by rendering an object in a HDR environment.
* LowLevelRendering - Demonstrates how to use the low-level rendering system to manually issue rendering commands. This is similar to using DirectX/OpenGL/Vulkan, except it uses bs::framework's platform-agnostic rendering layer.