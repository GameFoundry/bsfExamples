// Framework includes
#include "BsApplication.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Material/BsMaterial.h"
#include "Components/BsCCamera.h"
#include "Components/BsCRenderable.h"
#include "Components/BsCSkybox.h"
#include "Components/BsCPlaneCollider.h"
#include "Components/BsCBoxCollider.h"
#include "Components/BsCSphereCollider.h"
#include "Components/BsCCharacterController.h"
#include "Components/BsCRigidbody.h"
#include "GUI/BsCGUIWidget.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUILabel.h"
#include "Physics/BsPhysicsMaterial.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Scene/BsSceneObject.h"
#include "Platform/BsCursor.h"
#include "Input/BsInput.h"

// Example includes
#include "BsExampleFramework.h"
#include "BsFPSWalker.h"
#include "BsFPSCamera.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This example sets up a physical environment in which the user can walk around using the character controller component,
// and shoot the placed geometry demonstrating various aspects of the physics system. This includes a demonstration of
// static colliders, dynamic rigidbodies, physical materials, character controller and manual application of forces.
//
// The example first loads necessary resources, including textures, materialss and physical materials. Then it sets up the
// scene, consisting of a floor, and multiple stacks of boxes that can be knocked down. Character controller is created
// next, as well as the camera. Components for moving the character controller and the camera are attached to allow the
// user to control the character. Finally an input callback is hooked up that shoots spheres when user presses the left
// mouse button.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace bs
{
	constexpr float GROUND_PLANE_SCALE = 50.0f;

	u32 windowResWidth = 1280;
	u32 windowResHeight = 720;

	/** Set up the scene used by the example, and the camera to view the world through. */
	void setUpScene()
	{
		/************************************************************************/
		/* 									ASSETS	                    		*/
		/************************************************************************/

		// Prepare all the resources we'll be using throughout this example

		// Grab a couple of test textures that we'll apply to the rendered objects
		HTexture gridPattern = ExampleFramework::LoadTexture(ExampleTexture::GridPattern);
		HTexture gridPattern2 = ExampleFramework::LoadTexture(ExampleTexture::GridPattern2);

		// Grab the default PBR shader
		HShader shader = gBuiltinResources().GetBuiltinShader(BuiltinShader::Standard);

		// Create a set of materials to apply to renderables used
		HMaterial planeMaterial = Material::Create(shader);
		planeMaterial->SetTexture("gAlbedoTex", gridPattern2);

		// Tile the texture so every tile covers a 2x2m area
		planeMaterial->SetVec2("gUVTile", Vector2::ONE * GROUND_PLANE_SCALE * 0.5f);

		HMaterial boxMaterial = Material::Create(shader);
		boxMaterial->SetTexture("gAlbedoTex", gridPattern);

		HMaterial sphereMaterial = Material::Create(shader);

		// Load meshes we'll used for our rendered objects
		HMesh boxMesh = gBuiltinResources().GetMesh(BuiltinMesh::Box);
		HMesh planeMesh = gBuiltinResources().GetMesh(BuiltinMesh::Quad);
		HMesh sphereMesh = gBuiltinResources().GetMesh(BuiltinMesh::Sphere);

		// Create a physics material we'll use for the box geometry, as well as the floor. The material has high
		// static and dynamic friction, with low restitution (low bounciness). Simulates a harder, rough, solid surface.
		HPhysicsMaterial boxPhysicsMaterial = PhysicsMaterial::Create(1.0f, 1.0f, 0.0f);

		// Create a physics material for the sphere geometry, with higher bounciness. Simulates elasticity.
		HPhysicsMaterial spherePhysicsMaterial = PhysicsMaterial::Create(1.0f, 1.0f, 0.5f);

		/************************************************************************/
		/* 									FLOOR	                    		*/
		/************************************************************************/

		// Set up renderable geometry for the floor plane
		HSceneObject floorSO = SceneObject::Create("Floor");
		HRenderable floorRenderable = floorSO->AddComponent<CRenderable>();
		floorRenderable->SetMesh(planeMesh);
		floorRenderable->SetMaterial(planeMaterial);

		floorSO->SetScale(Vector3(GROUND_PLANE_SCALE, 1.0f, GROUND_PLANE_SCALE));

		// Add a plane collider that will prevent physical objects going through the floor
		HPlaneCollider planeCollider = floorSO->AddComponent<CPlaneCollider>();

		// Apply the non-bouncy material
		planeCollider->SetMaterial(boxPhysicsMaterial);

		/************************************************************************/
		/* 									BOXES	                    		*/
		/************************************************************************/

		// Helper method that creates a pyramid of six boxes that can be physically manipulated
		auto createBoxStack = [=](const Vector3& position, const Quaternion& rotation = Quaternion::IDENTITY)
		{
			HSceneObject boxSO[6];
			for(auto& entry : boxSO)
			{
				// Create a scene object and a renderable
				entry = SceneObject::Create("Box");

				HRenderable boxRenderable = entry->AddComponent<CRenderable>();
				boxRenderable->SetMesh(boxMesh);
				boxRenderable->SetMaterial(boxMaterial);

				// Add a plane collider that represent's the physical geometry of the box
				HBoxCollider boxCollider = entry->AddComponent<CBoxCollider>();

				// Apply the non-bouncy material
				boxCollider->SetMaterial(boxPhysicsMaterial);

				// Set the mass of a box to 25 kilograms
				boxCollider->SetMass(25.0f);

				// Add a rigidbody, making the box geometry able to react to interactions with other physical objects
				HRigidbody boxRigidbody = entry->AddComponent<CRigidbody>();
			}

			// Stack the boxes in a pyramid
			Vector3 positions[] = {
				// First row
				Vector3(-1.25f, 0.55f, 0.0f),
				Vector3(0.0f, 0.55f, 0.0f),
				Vector3(1.25f, 0.55f, 0.0f),
				// Second row
				Vector3(-0.65f, 1.6f, 0.0f),
				Vector3(0.65f, 1.6f, 0.0f),
				// Third row
				Vector3(0.0f, 2.65f, 0.0f),
			};

			for(u32 i = 0; i < 6; i++)
			{
				Vector3 pos = rotation.Rotate(positions[i]) + position;
				boxSO[i]->SetPosition(pos);
			}
		};

		createBoxStack(Vector3::ZERO);
		createBoxStack(Vector3(6.0f, 0.0f, 3.0f), Quaternion(Degree(0.0f), Degree(-45.0f), Degree(0.0f)));
		createBoxStack(Vector3(-6.0f, 0.0f, 3.0f), Quaternion(Degree(0.0f), Degree(45.0f), Degree(0.0f)));

		/************************************************************************/
		/* 									CHARACTER                    		*/
		/************************************************************************/

		// Add physics geometry and components for character movement and physics interaction
		HSceneObject characterSO = SceneObject::Create("Character");
		characterSO->SetPosition(Vector3(0.0f, 1.0f, 5.0f));

		// Add a character controller, representing the physical geometry of the character
		HCharacterController charController = characterSO->AddComponent<CCharacterController>();

		// Make the character about 1.8m high, with 0.4m radius (controller represents a capsule)
		charController->SetHeight(1.0f); // + 0.4 * 2 radius = 1.8m height
		charController->SetRadius(0.4f);

		// FPS walker uses default input controls to move the character controller attached to the same object
		characterSO->AddComponent<FPSWalker>();

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

		// Add a component that allows the camera to be rotated using the mouse
		HFPSCamera fpsCamera = sceneCameraSO->AddComponent<FPSCamera>();

		// Set the character controller on the FPS camera, so the component can apply yaw rotation to it
		fpsCamera->SetCharacter(characterSO);

		// Make the camera a child of the character scene object, and position it roughly at eye level
		sceneCameraSO->SetParent(characterSO);
		sceneCameraSO->SetPosition(Vector3(0.0f, 1.8f * 0.5f - 0.1f, 0.0f));

		/************************************************************************/
		/* 									SKYBOX                       		*/
		/************************************************************************/

		// Load a skybox texture
		HTexture skyCubemap = ExampleFramework::LoadTexture(ExampleTexture::EnvironmentDaytime, false, true, true);

		// Add a skybox texture for sky reflections
		HSceneObject skyboxSO = SceneObject::Create("Skybox");

		HSkybox skybox = skyboxSO->AddComponent<CSkybox>();
		skybox->SetTexture(skyCubemap);

		/************************************************************************/
		/* 									CURSOR                       		*/
		/************************************************************************/

		// Hide and clip the cursor, since we only use the mouse movement for camera rotation
		Cursor::Instance().Hide();
		Cursor::Instance().ClipToWindow(*window);

		/************************************************************************/
		/* 									INPUT                       		*/
		/************************************************************************/

		// Hook up input that launches a sphere when user clicks the mouse, and Esc key to quit
		gInput().OnButtonUp.Connect([=](const ButtonEvent& ev)
									{
			if(ev.ButtonCode == BC_MOUSE_LEFT)
			{
				// Create the scene object and renderable geometry of the sphere
				HSceneObject sphereSO = SceneObject::Create("Sphere");

				HRenderable sphereRenderable = sphereSO->AddComponent<CRenderable>();
				sphereRenderable->SetMesh(sphereMesh);
				sphereRenderable->SetMaterial(sphereMaterial);

				// Create a spherical collider, represting physical geometry
				HSphereCollider sphereCollider = sphereSO->AddComponent<CSphereCollider>();

				// Apply the bouncy material
				sphereCollider->SetMaterial(spherePhysicsMaterial);

				// Set mass to 25kg
				sphereCollider->SetMass(25.0f);

				// Add a rigidbody, making the object interactable
				HRigidbody sphereRigidbody = sphereSO->AddComponent<CRigidbody>();
				
				// Position the sphere in front of the character, and scale it down a bit
				Vector3 spawnPos = characterSO->GetTransform().GetPosition();
				spawnPos += sceneCameraSO->GetTransform().GetForward() * 0.5f;
				spawnPos.Y += 0.5f;

				sphereSO->SetPosition(spawnPos);
				sphereSO->SetScale(Vector3(0.3f, 0.3f, 0.3f));

				// Apply force to the sphere, launching it forward in the camera's view direction
				sphereRigidbody->AddForce(sceneCameraSO->GetTransform().GetForward() * 40.0f, ForceMode::Velocity);
			}
			else if(ev.ButtonCode == BC_ESCAPE)
			{
				// Quit the application when Escape key is pressed
				gApplication().QuitRequested();
			} });

		/************************************************************************/
		/* 									GUI		                     		*/
		/************************************************************************/

		// Display GUI elements indicating to the user which input keys are available

		// Add a GUIWidget component we will use for rendering the GUI
		HSceneObject guiSO = SceneObject::Create("GUI");
		HGUIWidget gui = guiSO->AddComponent<CGUIWidget>(sceneCamera);

		// Grab the main panel onto which to attach the GUI elements to
		GUIPanel* mainPanel = gui->GetPanel();

		// Create a vertical GUI layout to align the labels one below each other
		GUILayoutY* vertLayout = GUILayoutY::Create();

		// Create the GUI labels displaying the available input commands
		HString shootString(u8"Press left mouse button to shoot");
		HString quitString(u8"Press the Escape key to quit");

		vertLayout->AddNewElement<GUILabel>(shootString);
		vertLayout->AddNewElement<GUILabel>(quitString);

		// Register the layout with the main GUI panel, placing the layout in top left corner of the screen by default
		mainPanel->AddElement(vertLayout);
	}
} // namespace bs

/** Main entry point into the application. */
#if BS_PLATFORM == BS_PLATFORM_WIN32
#	include <windows.h>

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
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

	// Set up the scene with an object to render and a camera
	setUpScene();

	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::Instance().RunMainLoop();

	// When done, clean up
	Application::ShutDown();

	return 0;
}
