// Engine includes
#include "BsApplication.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Importer/BsImporter.h"
#include "Importer/BsTextureImportOptions.h"
#include "Importer/BsMeshImportOptions.h"
#include "Material/BsMaterial.h"
#include "Input/BsVirtualInput.h"
#include "Components/BsCCamera.h"
#include "Components/BsCRenderable.h"
#include "Components/BsCSkybox.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Scene/BsSceneObject.h"

// Example includes
#include "BsExampleConfig.h"
#include "BsCameraFlyer.h"
#include "BsObjectRotator.h"
#include "BsExampleFramework.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <windows.h>
#endif

namespace bs
{
	struct Assets;

	UINT32 windowResWidth = 1280;
	UINT32 windowResHeight = 720;

	/** Imports all of our assets and prepares GameObject that handle the example logic. */
	void setUpExample();

	/** Import mesh & textures used by the example. */
	void loadAssets(Assets& assets);

	/** Create a material used by our example model. */
	void createMaterial(Assets& assets);

	/** Set up example scene objects. */
	void setUp3DScene(const Assets& assets);

	/** Set up input configuration and callbacks. */
	void setUpInput();

	/** Triggered whenever a virtual button is released. */
	void buttonUp(const VirtualButton& button, UINT32 deviceIdx);
}

using namespace bs;

/** Main entry point into the application. */
#if BS_PLATFORM == BS_PLATFORM_WIN32
int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
#else
int main()
#endif
{
	// Initializes the application and creates a window with the specified properties
	VideoMode videoMode(windowResWidth, windowResHeight);
	Application::startUp(videoMode, "Example", false);

	// Imports all of ours assets and prepares GameObjects that handle the example logic.
	setUpExample();
	
	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::instance().runMainLoop();

	Application::shutDown();

	return 0;
}

namespace bs
{
	Path dataPath = EXAMPLE_DATA_PATH;
	Path exampleModelPath = dataPath + "Pistol/Pistol01.fbx";
	Path exampleAlbedoTexPath = dataPath + "Pistol/Pistol_DFS.png";
	Path exampleNormalsTexPath = dataPath + "Pistol/Pistol_NM.png";
	Path exampleRoughnessTexPath = dataPath + "Pistol/Pistol_RGH.png";
	Path exampleMetalnessTexPath = dataPath + "Pistol/Pistol_MTL.png";
	Path exampleSkyCubemapPath = dataPath + "Environments/PaperMill_E_3k.hdr";

	HCamera sceneCamera;

	/** Container for all resources used by the example. */
	struct Assets
	{
		HMesh exampleModel;
		HTexture exampleAlbedoTex;
		HTexture exampleNormalsTex;
		HTexture exampleRoughnessTex;
		HTexture exampleMetalnessTex;
		HTexture exampleSkyCubemap;
		HShader exampleShader;
		HMaterial exampleMaterial;
	};

	void setUpExample()
	{
		Assets assets;
		loadAssets(assets);
		createMaterial(assets);

		setUp3DScene(assets);
		setUpInput();
	}

	/**
	 * Load the required resources. First try to load a pre-processed version of the resources. If they don't exist import
	 * resources from the source formats into engine format, and save them for next time.
	 */
	void loadAssets(Assets& assets)
	{
		// Load an FBX mesh.
		assets.exampleModel = ExampleFramework::loadMesh(exampleModelPath, 10.0f);

		// Load textures
		assets.exampleAlbedoTex = ExampleFramework::loadTexture(exampleAlbedoTexPath);
		assets.exampleNormalsTex = ExampleFramework::loadTexture(exampleNormalsTexPath, false);
		assets.exampleRoughnessTex = ExampleFramework::loadTexture(exampleRoughnessTexPath, false);
		assets.exampleMetalnessTex = ExampleFramework::loadTexture(exampleMetalnessTexPath, false);
		assets.exampleSkyCubemap = ExampleFramework::loadTexture(exampleSkyCubemapPath, false, true, true);

		// Load the default physically based shader for rendering opaque objects
		assets.exampleShader = BuiltinResources::instance().getBuiltinShader(BuiltinShader::Standard);
	}

	/** Create a material using the active shader, and assign the relevant textures to it. */
	void createMaterial(Assets& assets)
	{
		// Create a material with the active shader.
		HMaterial exampleMaterial = Material::create(assets.exampleShader);

		// Assign the four textures requires by the PBS shader
		exampleMaterial->setTexture("gAlbedoTex", assets.exampleAlbedoTex);
		exampleMaterial->setTexture("gNormalTex", assets.exampleNormalsTex);
		exampleMaterial->setTexture("gRoughnessTex", assets.exampleRoughnessTex);
		exampleMaterial->setTexture("gMetalnessTex", assets.exampleMetalnessTex);

		assets.exampleMaterial = exampleMaterial;
	}

	/** Set up the 3D object used by the example, and the camera to view the world through. */
	void setUp3DScene(const Assets& assets)
	{
		/************************************************************************/
		/* 								SCENE OBJECT                      		*/
		/************************************************************************/

		// Now we create a scene object that has a position, orientation, scale and optionally
		// components to govern its logic. In this particular case we are creating a SceneObject
		// with a Renderable component which will render a mesh at the position of the scene object
		// with the provided material.

		// Create new scene object at (0, 0, 0)
		HSceneObject pistolSO = SceneObject::create("Pistol");
		
		// Attach the Renderable component and hook up the mesh we imported earlier,
		// and the material we created in the previous section.
		HRenderable renderable = pistolSO->addComponent<CRenderable>();
		renderable->setMesh(assets.exampleModel);
		renderable->setMaterial(assets.exampleMaterial);

		// Add a rotator component so we can rotate the object during runtime
		pistolSO->addComponent<ObjectRotator>();

		/************************************************************************/
		/* 									SKYBOX                       		*/
		/************************************************************************/

		// Add a skybox texture for sky reflections
		HSceneObject skyboxSO = SceneObject::create("Skybox");

		HSkybox skybox = skyboxSO->addComponent<CSkybox>();
		skybox->setTexture(assets.exampleSkyCubemap);

		/************************************************************************/
		/* 									CAMERA	                     		*/
		/************************************************************************/

		// In order something to render on screen we need at least one camera.

		// Like before, we create a new scene object at (0, 0, 0).
		HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");

		// Get the primary render window we need for creating the camera. 
		SPtr<RenderWindow> window = gApplication().getPrimaryWindow();

		// Add a Camera component that will output whatever it sees into that window 
		// (You could also use a render texture or another window you created).
		sceneCamera = sceneCameraSO->addComponent<CCamera>();
		sceneCamera->getViewport()->setTarget(window);

		// Set up camera component properties

		// Set closest distance that is visible. Anything below that is clipped.
		sceneCamera->setNearClipDistance(0.005f);

		// Set farthest distance that is visible. Anything above that is clipped.
		sceneCamera->setFarClipDistance(1000);

		// Set aspect ratio depending on the current resolution
		sceneCamera->setAspectRatio(windowResWidth / (float)windowResHeight);

		// Enable multi-sample anti-aliasing for better quality
		sceneCamera->setMSAACount(4);

		// Add a CameraFlyer component that allows us to move the camera. See CameraFlyer for more information.
		sceneCameraSO->addComponent<CameraFlyer>();

		// Position and orient the camera scene object
		sceneCameraSO->setPosition(Vector3(2.0f, 1.0f, 2.0f));
		sceneCameraSO->lookAt(Vector3(-0.4f, 0, 0));
	}

	/** Register mouse and keyboard inputs that will be used for controlling the camera. */
	void setUpInput()
	{
		// Register input configuration
		// Banshee allows you to use VirtualInput system which will map input device buttons
		// and axes to arbitrary names, which allows you to change input buttons without affecting
		// the code that uses it, since the code is only aware of the virtual names. 
		// If you want more direct input, see Input class.
		auto inputConfig = VirtualInput::instance().getConfiguration();

		// Camera controls for buttons (digital 0-1 input, e.g. keyboard or gamepad button)
		inputConfig->registerButton("Forward", BC_W);
		inputConfig->registerButton("Back", BC_S);
		inputConfig->registerButton("Left", BC_A);
		inputConfig->registerButton("Right", BC_D);
		inputConfig->registerButton("Forward", BC_UP);
		inputConfig->registerButton("Back", BC_DOWN);
		inputConfig->registerButton("Left", BC_LEFT);
		inputConfig->registerButton("Right", BC_RIGHT);
		inputConfig->registerButton("FastMove", BC_LSHIFT);
		inputConfig->registerButton("RotateObj", BC_MOUSE_LEFT);
		inputConfig->registerButton("RotateCam", BC_MOUSE_RIGHT);

		// Camera controls for axes (analog input, e.g. mouse or gamepad thumbstick)
		// These return values in [-1.0, 1.0] range.
		inputConfig->registerAxis("Horizontal", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseX));
		inputConfig->registerAxis("Vertical", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseY));
	}
}

