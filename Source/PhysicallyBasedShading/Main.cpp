// Framework includes
#include "BsApplication.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Material/BsMaterial.h"
#include "Components/BsCCamera.h"
#include "Components/BsCRenderable.h"
#include "Components/BsCSkybox.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Scene/BsSceneObject.h"

// Example includes
#include "BsObjectRotator.h"
#include "BsExampleFramework.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This example renders an object using the standard built-in physically based material. 
//
// The example first loads necessary resources, including a mesh and textures to use for rendering. Then it creates a
// material using the standard PBR shader. It then proceeds to register the relevant keys used for controling the camera
// and the rendered object. Finally it sets up the 3D scene using the mesh, textures, material and sets up a camera, along
// with CameraFlyer and ObjectRotator components that allow the user to fly around the scene and rotate the 3D model.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace bs
{
	UINT32 windowResWidth = 1280;
	UINT32 windowResHeight = 720;

	/** Container for all resources used by the example. */
	struct Assets
	{
		HMesh exampleModel;
		HTexture exampleAlbedoTex;
		HTexture exampleNormalsTex;
		HTexture exampleRoughnessTex;
		HTexture exampleMetalnessTex;
		HTexture exampleSkyCubemap;
		HMaterial exampleMaterial;
	};

	/** Load the resources we'll be using throughout the example. */
	Assets loadAssets()
	{
		Assets assets;

		// Load a 3D model
		assets.exampleModel = ExampleFramework::loadMesh(ExampleMesh::Cerberus);

		// Load PBR textures for the 3D model
		assets.exampleAlbedoTex = ExampleFramework::loadTexture(ExampleTexture::CerberusAlbedo);
		assets.exampleNormalsTex = ExampleFramework::loadTexture(ExampleTexture::CerberusNormal, false);
		assets.exampleRoughnessTex = ExampleFramework::loadTexture(ExampleTexture::CerberusRoughness, false);
		assets.exampleMetalnessTex = ExampleFramework::loadTexture(ExampleTexture::CerberusMetalness, false);

		// Create a material using the default physically based shader, and apply the PBR textures we just loaded
		HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);
		assets.exampleMaterial = Material::create(shader);

		assets.exampleMaterial->setTexture("gAlbedoTex", assets.exampleAlbedoTex);
		assets.exampleMaterial->setTexture("gNormalTex", assets.exampleNormalsTex);
		assets.exampleMaterial->setTexture("gRoughnessTex", assets.exampleRoughnessTex);
		assets.exampleMaterial->setTexture("gMetalnessTex", assets.exampleMetalnessTex);

		// Load an environment map
		assets.exampleSkyCubemap = ExampleFramework::loadTexture(ExampleTexture::EnvironmentPaperMill, false, true, true);

		return assets;
	}

	/** Set up the 3D object used by the example, and the camera to view the world through. */
	void setUp3DScene(const Assets& assets)
	{
		/************************************************************************/
		/* 									RENDERABLE                  		*/
		/************************************************************************/

		// Now we create a scene object that has a position, orientation, scale and optionally components to govern its 
		// logic. In this particular case we are creating a SceneObject with a Renderable component which will render a
		// mesh at the position of the scene object with the provided material.

		// Create new scene object at (0, 0, 0)
		HSceneObject pistolSO = SceneObject::create("Pistol");
		
		// Attach the Renderable component and hook up the mesh we loaded, and the material we created.
		HRenderable renderable = pistolSO->addComponent<CRenderable>();
		renderable->setMesh(assets.exampleModel);
		renderable->setMaterial(assets.exampleMaterial);

		pistolSO->setRotation(Quaternion(Degree(0.0f), Degree(-160.0f), Degree(0.0f)));

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
		HCamera sceneCamera = sceneCameraSO->addComponent<CCamera>();
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

		// Position and orient the camera scene object
		sceneCameraSO->setPosition(Vector3(0.2f, 0.05f, 1.4f));
		sceneCameraSO->lookAt(Vector3(0.2f, 0.05f, 0.0f));
	}
}

/** Main entry point into the application. */
#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <windows.h>

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
	using namespace bs;

	// Initializes the application and creates a window with the specified properties
	VideoMode videoMode(windowResWidth, windowResHeight);
	Application::startUp(videoMode, "Example", false);

	// Registers a default set of input controls
	ExampleFramework::setupInputConfig();

	// Load a model and textures, create materials
	Assets assets = loadAssets();

	// Set up the scene with an object to render and a camera
	setUp3DScene(assets);
	
	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::instance().runMainLoop();

	// When done, clean up
	Application::shutDown();

	return 0;
}