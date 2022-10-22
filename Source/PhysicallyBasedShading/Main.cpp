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
	u32 windowResWidth = 1280;
	u32 windowResHeight = 720;

	/** Container for all resources used by the example. */
	struct Assets 
	{
		HMesh ExampleModel;
		HTexture ExampleAlbedoTex;
		HTexture ExampleNormalsTex;
		HTexture ExampleRoughnessTex;
		HTexture ExampleMetalnessTex;
		HTexture ExampleSkyCubemap;
		HMaterial ExampleMaterial;
	};

	/** Load the resources we'll be using throughout the example. */
	Assets loadAssets()
	{
		Assets assets;

		// Load a 3D model
		assets.ExampleModel = ExampleFramework::LoadMesh(ExampleMesh::Cerberus);

		// Load PBR textures for the 3D model
		assets.ExampleAlbedoTex = ExampleFramework::LoadTexture(ExampleTexture::CerberusAlbedo);
		assets.ExampleNormalsTex = ExampleFramework::LoadTexture(ExampleTexture::CerberusNormal, false);
		assets.ExampleRoughnessTex = ExampleFramework::LoadTexture(ExampleTexture::CerberusRoughness, false);
		assets.ExampleMetalnessTex = ExampleFramework::LoadTexture(ExampleTexture::CerberusMetalness, false);

		// Create a material using the default physically based shader, and apply the PBR textures we just loaded
		HShader shader = gBuiltinResources().GetBuiltinShader(BuiltinShader::Standard);
		assets.ExampleMaterial = Material::Create(shader);

		assets.ExampleMaterial->SetTexture("gAlbedoTex", assets.ExampleAlbedoTex);
		assets.ExampleMaterial->SetTexture("gNormalTex", assets.ExampleNormalsTex);
		assets.ExampleMaterial->SetTexture("gRoughnessTex", assets.ExampleRoughnessTex);
		assets.ExampleMaterial->SetTexture("gMetalnessTex", assets.ExampleMetalnessTex);

		// Load an environment map
		assets.ExampleSkyCubemap = ExampleFramework::LoadTexture(ExampleTexture::EnvironmentPaperMill, false, true, true);

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
		HSceneObject pistolSO = SceneObject::Create("Pistol");
		
		// Attach the Renderable component and hook up the mesh we loaded, and the material we created.
		HRenderable renderable = pistolSO->AddComponent<CRenderable>();
		renderable->SetMesh(assets.ExampleModel);
		renderable->SetMaterial(assets.ExampleMaterial);

		pistolSO->SetRotation(Quaternion(Degree(0.0f), Degree(-160.0f), Degree(0.0f)));

		// Add a rotator component so we can rotate the object during runtime
		pistolSO->AddComponent<ObjectRotator>();

		/************************************************************************/
		/* 									SKYBOX                       		*/
		/************************************************************************/

		// Add a skybox texture for sky reflections
		HSceneObject skyboxSO = SceneObject::Create("Skybox");

		HSkybox skybox = skyboxSO->AddComponent<CSkybox>();
		skybox->SetTexture(assets.ExampleSkyCubemap);

		/************************************************************************/
		/* 									CAMERA	                     		*/
		/************************************************************************/

		// In order something to render on screen we need at least one camera.

		// Like before, we create a new scene object at (0, 0, 0).
		HSceneObject sceneCameraSO = SceneObject::Create("SceneCamera");

		// Get the primary render window we need for creating the camera. 
		SPtr<RenderWindow> window = gApplication().GetPrimaryWindow();

		// Add a Camera component that will output whatever it sees into that window 
		// (You could also use a render texture or another window you created).
		HCamera sceneCamera = sceneCameraSO->AddComponent<CCamera>();
		sceneCamera->GetViewport()->SetTarget(window);

		// Set up camera component properties

		// Set closest distance that is visible. Anything below that is clipped.
		sceneCamera->SetNearClipDistance(0.005f);

		// Set farthest distance that is visible. Anything above that is clipped.
		sceneCamera->SetFarClipDistance(1000);

		// Set aspect ratio depending on the current resolution
		sceneCamera->SetAspectRatio(windowResWidth / (float)windowResHeight);

		// Position and orient the camera scene object
		sceneCameraSO->SetPosition(Vector3(0.2f, 0.05f, 1.4f));
		sceneCameraSO->LookAt(Vector3(0.2f, 0.05f, 0.0f));
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
	Application::StartUp(videoMode, "Example", false);

	// Registers a default set of input controls
	ExampleFramework::SetupInputConfig();

	// Load a model and textures, create materials
	Assets assets = loadAssets();

	// Set up the scene with an object to render and a camera
	setUp3DScene(assets);
	
	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::Instance().RunMainLoop();

	// When done, clean up
	Application::ShutDown();

	return 0;
}
