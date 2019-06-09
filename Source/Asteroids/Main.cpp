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
#include "Components/BsCCharacterController.h"
#include "Components/BsCRigidbody.h"
#include "Physics/BsPhysicsMaterial.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Scene/BsSceneObject.h"
#include "Input/BsInput.h"
#include "Components/BsCDecal.h"

// Example includes
#include "BsExampleFramework.h"
#include "BsFPSWalker.h"
#include "BsFPSCamera.h"
#include "Math/BsRandom.h"

#include "./mesh.h"
#include "./CSpinner.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This example sets up a simple environment consisting of a floor and cube, and a decal projecting on both surfaces. The
// example demonstrates how to set up decals, how decals are not shown on surfaces perpendicular to the decal direction,
// and optionally how to use masking to only project a decal onto a certain set of surfaces.
//
// It also sets up necessary physical objects for collision, as well as the character collider and necessary components
// for walking around the environment.
//
// The example first sets up the scene consisting of a floor, box and a skybox. Character controller is created next,
// as well as the camera. Components for moving the character controller and the camera are attached to allow the user to
// control the character. It then loads the required decal textures, sets up a decal material and initializes the actual
// decal component. Finally the cursor is hidden and quit on Esc key press hooked up.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace bs
{
  constexpr float GROUND_PLANE_SCALE = 50.0f;

  UINT32 windowResWidth = 1280;
  UINT32 windowResHeight = 720;

  SPtr<Decal> decal;

  void addCamera(HSceneObject characterSO) {
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

    // Add a component that allows the camera to be rotated using the mouse
    sceneCameraSO->setRotation(Quaternion(Degree(-10.0f), Degree(0.0f), Degree(0.0f)));
    HFPSCamera fpsCamera = sceneCameraSO->addComponent<FPSCamera>();

    // Set the character controller on the FPS camera, so the component can apply yaw rotation to it
    fpsCamera->setCharacter(characterSO);

    // Make the camera a child of the character scene object, and position it roughly at eye level
    sceneCameraSO->setParent(characterSO);
    sceneCameraSO->setPosition(Vector3(0.0f, 1.8f * 0.5f - 0.1f, -2.0f));
  }

  void addFloor() {
    /************************************************************************/
    /*                  ASSETS                          */
    /************************************************************************/

    // Prepare all the resources we'll be using throughout this example

    // Grab a couple of test textures that we'll apply to the rendered objects
    HTexture gridPattern = ExampleFramework::loadTexture(ExampleTexture::GridPattern);
    HTexture gridPattern2 = ExampleFramework::loadTexture(ExampleTexture::GridPattern2);
    // Grab the default PBR shader
    HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);

    // Create a set of materials to apply to renderables used
    HMaterial planeMaterial = Material::create(shader);
    planeMaterial->setTexture("gAlbedoTex", gridPattern2);

    // Tile the texture so every tile covers a 2x2m area
    planeMaterial->setVec2("gUVTile", Vector2::ONE * GROUND_PLANE_SCALE * 0.5f);

    // Load meshes we'll used for our rendered objects
    HMesh planeMesh = gBuiltinResources().getMesh(BuiltinMesh::Quad);

    // Set up renderable geometry for the floor plane
    HSceneObject floorSO = SceneObject::create("Floor");
    HRenderable floorRenderable = floorSO->addComponent<CRenderable>();
    floorRenderable->setMesh(planeMesh);
    floorRenderable->setMaterial(planeMaterial);

    floorSO->setScale(Vector3(GROUND_PLANE_SCALE, 1.0f, GROUND_PLANE_SCALE));

    // Add a plane collider that will prevent physical objects going through the floor
    HPlaneCollider planeCollider = floorSO->addComponent<CPlaneCollider>();

  }

  void addAsteroids() {

    // make asteroid meshes.
    unsigned int meshCount = 100;
    unsigned int subdivCount = 3;

    HShader shader = gBuiltinResources().getBuiltinShader(BuiltinShader::Standard);
    HMaterial asteroidMaterial = Material::create(shader);

    std::vector<HMesh> meshes;
    makeMeshes(meshCount, subdivCount, meshes);
    assert(meshes.size() == meshCount);

    Random rand;

    for (unsigned int i = 0; i < meshes.size(); ++i) {
      auto mesh = meshes[i];
      HSceneObject ast = SceneObject::create("Ast");
      HRenderable boxRenderable = ast->addComponent<CRenderable>();
      ast->addComponent<CSpinner>(&rand);

      boxRenderable->setMesh(mesh);
      boxRenderable->setMaterial(asteroidMaterial);

      ast->setPosition(Vector3(i * 3, 1.2f, -10.5f));
    }

    unsigned int numInstances = 1000; // 10,000
    for (unsigned int i = 0; i < numInstances; ++i) {
      auto mesh = meshes[i % meshCount];
      HSceneObject ast = SceneObject::create("RevolvingAst");
      HRenderable boxRenderable = ast->addComponent<CRenderable>();
      ast->addComponent<CSpinner>(&rand);
      ast->addComponent<COrbiter>(&rand);

      boxRenderable->setMesh(mesh);
      boxRenderable->setMaterial(asteroidMaterial);

      float shellThickness = 0.5; // 10% outer sphere.
      Vector3 point = rand.getPointInSphereShell(shellThickness);
      point *= 100;
      ast->setPosition(point);
    }

  }

  /** Set up the scene used by the example, and the camera to view the world through. */
  void setUpScene()
  {


    addAsteroids();

    /************************************************************************/
    /*                  FLOOR                         */
    /************************************************************************/

    addFloor();

    /************************************************************************/
    /*                  CHARACTER                       */
    /************************************************************************/

    // Add physics geometry and components for character movement and physics interaction
    HSceneObject characterSO = SceneObject::create("Character");
    characterSO->setPosition(Vector3(0.0f, 1.0f, 5.0f));

    // Add a character controller, representing the physical geometry of the character
    HCharacterController charController = characterSO->addComponent<CCharacterController>();

    // Make the character about 1.8m high, with 0.4m radius (controller represents a capsule)
    charController->setHeight(1.0f); // + 0.4 * 2 radius = 1.8m height
    charController->setRadius(0.4f);

    // FPS walker uses default input controls to move the character controller attached to the same object
    characterSO->addComponent<FPSWalker>();

    /************************************************************************/
    /*                  CAMERA                          */
    /************************************************************************/

    addCamera(characterSO);

    /************************************************************************/
    /*                  SKYBOX                          */
    /************************************************************************/

    // Load a skybox texture
    HTexture skyCubemap = ExampleFramework::loadTexture(ExampleTexture::EnvironmentRathaus, false, true, true);

    // Add a skybox texture for sky reflections
    HSceneObject skyboxSO = SceneObject::create("Skybox");

    HSkybox skybox = skyboxSO->addComponent<CSkybox>();
    skybox->setTexture(skyCubemap);

    /************************************************************************/
    /*                  INPUT                           */
    /************************************************************************/

    // Hook up input that launches a sphere when user clicks the mouse, and Esc key to quit
    gInput().onButtonUp.connect([=](const ButtonEvent& ev)
    {
      if(ev.buttonCode == BC_ESCAPE)
      {
        // Quit the application when Escape key is pressed
        gApplication().quitRequested();
      }
    });

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

  // Set up the scene with an object to render and a camera
  setUpScene();

  // Runs the main loop that does most of the work. This method will exit when user closes the main
  // window or exits in some other way.
  Application::instance().runMainLoop();

  // When done, clean up
  Application::shutDown();

  return 0;
}
