// Framework includes
#include "BsApplication.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Material/BsMaterial.h"
#include "Components/BsCCamera.h"
#include "Components/BsCRenderable.h"
#include "Components/BsCLight.h"
#include "Components/BsCSkybox.h"
#include "Components/BsCPlaneCollider.h"
#include "Components/BsCCharacterController.h"
#include "Components/BsCParticleSystem.h"
#include "Image/BsSpriteTexture.h"
#include "Particles/BsParticleSystem.h"
#include "Particles/BsParticleEmitter.h"
#include "Particles/BsParticleEvolver.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Scene/BsSceneObject.h"
#include "Platform/BsCursor.h"
#include "Input/BsInput.h"
#include "Utility/BsTime.h"

// Example includes
#include "BsExampleFramework.h"
#include "BsFPSWalker.h"
#include "BsFPSCamera.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This example sets up an environment with three particle systems:
// - Smoke effect using traditional billboard particles
// - 3D particles with support for world collisions and lighting
// - GPU particle simulation with a vector field
//
// It also sets up necessary physical objects for collision, as well as the character collider and necessary components
// for walking around the environment.
//
// The example first loads necessary resources, including textures and materials. Then it set up the scene, consisting of a
// floor and a skybox. Character controller is created next, as well as the camera. Components for moving the character 
// controller and the camera are attached to allow the user to control the character. Finally it sets up three separate
// particle systems, their creation wrapped in their own creation methods. Finally the cursor is hidden and quit on Esc
// key press hooked up.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace bs
{
	constexpr float GROUND_PLANE_SCALE = 50.0f;

	u32 windowResWidth = 1280;
	u32 windowResHeight = 720;

	// Set up a helper component that makes the object its attached to orbit a point. This is used by the 3D particle
	// system for moving its light.
	class LightOrbit : public Component
	{
	public:
		LightOrbit(const HSceneObject& parent, float radius)
			:Component(parent), mRadius(radius)
		{ }

		void OnInitialized() override
		{
			mCenter = SO()->GetTransform().GetPosition();
		}

		void Update() override
		{
			Vector3 position = mCenter + mRadius * Vector3(Math::Cos(mAngle), 0.0f, Math::Sin(mAngle));

			mAngle += Degree(gTime().GetFrameDelta() * 90.0f);

			SO()->SetWorldPosition(position);
		}

	private:
		Degree mAngle = Degree(0.0f);
		Vector3 mCenter;
		float mRadius;
	};

	/** Container for all assets used by the particles systems in this example. */
	struct ParticleSystemAssets
	{
		// Smoke particle system assets
		HSpriteTexture SmokeTex;
		HMaterial SmokeMat;

		// 3D particle system assets
		HMesh SphereMesh;
		HMaterial Particles3DMat;
		HMaterial LightMat;

		// GPU particle system assets
		HMaterial LitParticleEmissiveMat;
		HVectorField VectorField;
	};

	/** Load the assets used by the particle systems. */
	ParticleSystemAssets loadParticleSystemAssets()
	{
		ParticleSystemAssets assets;

		// Smoke particle system assets
		//// Import the texture and set up a sprite texture so we can animate it
		HTexture smokeTex = ExampleFramework::LoadTexture(ExampleTexture::ParticleSmoke);
		assets.SmokeTex = SpriteTexture::Create(smokeTex);

		//// Set up sprite sheet animation on the sprite texture
		SpriteSheetGridAnimation smokeGridAnim(5, 6, 30, 30);
		assets.SmokeTex->SetAnimation(smokeGridAnim);
		assets.SmokeTex->SetAnimationPlayback(SpriteAnimationPlayback::None);

		//// Set up a shader without lighting and enable soft particle rendering
		HShader particleUnlitShader = gBuiltinResources().GetBuiltinShader(BuiltinShader::ParticlesUnlit);
		assets.SmokeMat = Material::Create(particleUnlitShader);
		assets.SmokeMat->SetVariation(ShaderVariation(
			{
				ShaderVariation::Param("SOFT", true)
			})
		);

		//// Fade over the range of 2m (used for soft particle blending)
		assets.SmokeMat->SetFloat("gInvDepthRange", 1.0f / 2.0f);
		assets.SmokeMat->SetSpriteTexture("gTexture", assets.SmokeTex);

		// Set up an emissive material used in the GPU vector field example
		HShader particleLitShader = gBuiltinResources().GetBuiltinShader(BuiltinShader::ParticlesLitOpaque);
		assets.LitParticleEmissiveMat = Material::Create(particleLitShader);
		assets.LitParticleEmissiveMat->SetTexture("gEmissiveMaskTex", gBuiltinResources().GetTexture(BuiltinTexture::White));
		assets.LitParticleEmissiveMat->SetColor("gEmissiveColor", Color::White * 10.0f);

		// 3D particle system assets
		//// Create another lit material using a plain white albedo texture
		assets.Particles3DMat = Material::Create(particleLitShader);
		assets.Particles3DMat->SetTexture("gAlbedoTex", gBuiltinResources().GetTexture(BuiltinTexture::White));

		//// Create a material used for rendering the light sphere itself
		HShader standardShader = gBuiltinResources().GetBuiltinShader(BuiltinShader::Standard);
		assets.LightMat = Material::Create(standardShader);
		assets.LightMat->SetTexture("gEmissiveMaskTex", gBuiltinResources().GetTexture(BuiltinTexture::White));
		assets.LightMat->SetColor("gEmissiveColor", Color::Red * 5.0f);

		//// Import a vector field used in the GPU simulation
		assets.VectorField = ExampleFramework::LoadResource<VectorField>(ExampleResource::VectorField);

		//// Import a sphere mesh used for the 3D particles and the light sphere
		assets.SphereMesh = gBuiltinResources().GetMesh(BuiltinMesh::Sphere);

		return assets;
	}

	void setupGPUParticleEffect(const Vector3& pos, const ParticleSystemAssets& assets);
	void setup3DParticleEffect(const Vector3& pos, const ParticleSystemAssets& assets);
	void setupSmokeEffect(const Vector3& pos, const ParticleSystemAssets& assets);

	/** Set up the scene used by the example, and the camera to view the world through. */
	void setUpScene()
	{
		/************************************************************************/
		/* 									ASSETS	                    		*/
		/************************************************************************/

		// Prepare the assets required for the scene and background

		// Grab a texture used for rendering the ground
		HTexture gridPattern = ExampleFramework::LoadTexture(ExampleTexture::GridPattern2);

		// Grab the default PBR shader
		HShader shader = gBuiltinResources().GetBuiltinShader(BuiltinShader::Standard);
		
		// Create a material for rendering the ground and apply the ground texture
		HMaterial planeMaterial = Material::Create(shader);
		planeMaterial->SetTexture("gAlbedoTex", gridPattern);

		// Tile the texture so every tile covers a 2x2m area
		planeMaterial->SetVec2("gUVTile", Vector2::ONE * GROUND_PLANE_SCALE * 0.5f);

		// Load the floor mesh
		HMesh planeMesh = gBuiltinResources().GetMesh(BuiltinMesh::Quad);

		// Load assets used by the particle systems
		ParticleSystemAssets assets = loadParticleSystemAssets();

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
		sceneCameraSO->SetRotation(Quaternion(Degree(-10.0f), Degree(0.0f), Degree(0.0f)));
		HFPSCamera fpsCamera = sceneCameraSO->AddComponent<FPSCamera>();

		// Set the character controller on the FPS camera, so the component can apply yaw rotation to it
		fpsCamera->SetCharacter(characterSO);

		// Make the camera a child of the character scene object, and position it roughly at eye level
		sceneCameraSO->SetParent(characterSO);
		sceneCameraSO->SetPosition(Vector3(0.0f, 1.8f * 0.5f - 0.1f, -2.0f));

		// Enable Bloom effect so that emissive materials look better
		auto rs = sceneCamera->GetRenderSettings();
		rs->Bloom.Enabled = true;
		rs->Bloom.Intensity = 0.1f;
		rs->Bloom.Threshold = 5.0f;
		rs->Bloom.Quality = 3;

		sceneCamera->SetRenderSettings(rs);

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
		/* 								PARTICLES                       		*/
		/************************************************************************/

		// Set up different particle systems
		setup3DParticleEffect(Vector3(-5.0f, 1.0f, 0.0f), assets);
		setupGPUParticleEffect(Vector3(0.0f, 1.0f, 0.0f), assets);
		setupSmokeEffect(Vector3(5.0f, 0.0f, 0.0f), assets);

		/************************************************************************/
		/* 									CURSOR                       		*/
		/************************************************************************/

		// Hide and clip the cursor, since we only use the mouse movement for camera rotation
		Cursor::Instance().Hide();
		Cursor::Instance().ClipToWindow(*window);

		/************************************************************************/
		/* 									INPUT                       		*/
		/************************************************************************/

		// Hook up Esc key to quit
		gInput().OnButtonUp.Connect([=](const ButtonEvent& ev)
		{
			if(ev.ButtonCode == BC_ESCAPE)
			{
				// Quit the application when Escape key is pressed
				gApplication().QuitRequested();
			}
		});
	}

	/** 
	 * Sets up a particle system using traditional billboard particles to render a smoke effect. The particles are emitted 
	 * from the base and distributed towards a cone shape. After emission particle color, size and velocity is modified
	 * through particle evolvers.
	 */
	void setupSmokeEffect(const Vector3& pos, const ParticleSystemAssets& assets)
	{
		// Create the particle system scene object and position/orient it
		HSceneObject particleSystemSO = SceneObject::Create("Smoke");
		particleSystemSO->SetPosition(pos);
		particleSystemSO->SetRotation(Quaternion(Degree(0), Degree(90), Degree(90)));

		// Add a particle system component
		HParticleSystem particleSystem = particleSystemSO->AddComponent<CParticleSystem>();

		// Set up the emitter
		SPtr<ParticleEmitter> emitter = bs_shared_ptr_new<ParticleEmitter>();

		// All newly spawned particles will have the size of 1m
		emitter->SetInitialSize(1.0f);

		// 20 particles will be emitted per second
		emitter->SetEmissionRate(20.0f);

		// Particles will initially move at a rate of 1m/s
		emitter->SetInitialSpeed(1.0f);

		// Particles will live for exactly 5 seconds
		emitter->SetInitialLifetime(5.0f);

		// Particles will initially have no tint
		emitter->SetInitialColor(Color::White);

		// Set up a shape that determines the position and initial travel direction of newly spawned particles. In this
		// case we're using a cone shape.
		PARTICLE_CONE_SHAPE_DESC coneShape;

		// All particles will spawn at the narrow point in the cone (position doesn't vary)
		coneShape.Type = ParticleEmitterConeType::Base;

		// The particle travel direction will be in the 10 degrees spawned by the cone
		coneShape.Angle = Degree(10.0f);

		// Assign the shape to the emitter
		emitter->SetShape(ParticleEmitterConeShape::Create(coneShape));

		// Assign the emitter to the particle system
		particleSystem->SetEmitters({emitter});

		// Set up evolvers that will modify the particle systems over its lifetime
		Vector<SPtr<ParticleEvolver>> evolvers;

		// Animate particle texture - this uses the sprite sheet animation set up during the asset loading step
		PARTICLE_TEXTURE_ANIMATION_DESC texAnimDesc;

		// Perform one animation cycle during the particle lifetime
		texAnimDesc.NumCycles = 1;

		// Create and add the texture animation evolver
		SPtr<ParticleEvolver> texAnimEvolver = bs_shared_ptr_new<ParticleTextureAnimation>(texAnimDesc);
		evolvers.push_back(texAnimEvolver);

		// Scale particles from size 1 to size 4 over their lifetime
		PARTICLE_SIZE_DESC sizeDesc;
		sizeDesc.Size = TAnimationCurve<float>(
			{
				TKeyframe<float>{1.0f, 0.0f, 1.0f, 0.0f},
				TKeyframe<float>{4.0f, 1.0f, 0.0f, 1.0f},
			});

		// Create and add the size evolver
		SPtr<ParticleEvolver> sizeEvolver = bs_shared_ptr_new<ParticleSize>(sizeDesc);
		evolvers.push_back(sizeEvolver);

		// Modify particle tint from white (no tint) to dark gray over first 40% of their lifetime
		PARTICLE_COLOR_DESC colorDesc;
		colorDesc.Color = ColorGradient(
			{
				ColorGradientKey(Color::White, 0.0f),
				ColorGradientKey(Color(0.1f, 0.1f, 0.1f, 1.0f), 0.4f)
			}
		);

		// Create and add the color evolver
		SPtr<ParticleEvolver> colorEvolver = bs_shared_ptr_new<ParticleColor>(colorDesc);
		evolvers.push_back(colorEvolver);

		// Apply force moving the particles to the right
		PARTICLE_FORCE_DESC forceDesc;
		forceDesc.Force = TAnimationCurve<Vector3>(
			{
				TKeyframe<Vector3>{Vector3::ZERO, Vector3::ZERO, Vector3::ONE, 0.0f},
				TKeyframe<Vector3>{Vector3(100.0f, 0.0f, 0.0f), -Vector3::ONE, Vector3::ZERO, 0.5f},
			});

		// Lets the system know the provided force direction is in world space
		forceDesc.WorldSpace = true;

		// Create and add the force evolver
		SPtr<ParticleEvolver> forceEvolver = bs_shared_ptr_new<ParticleForce>(forceDesc);
		evolvers.push_back(forceEvolver);

		// Register all the evolvers with the particle system
		particleSystem->SetEvolvers(evolvers);

		// Set up general particle system settings
		ParticleSystemSettings psSettings;

		// Orient the particles towards the camera plane (standard for billboard particles)
		psSettings.Orientation = ParticleOrientation::ViewPlane;

		// But lock the Y orientation
		psSettings.OrientationLockY = true;

		// Sort based on distance from the camera so that transparency looks appropriate
		psSettings.SortMode = ParticleSortMode::Distance;

		// Assign the material we created earlier
		psSettings.Material = assets.SmokeMat;

		// And actually apply the settings
		particleSystem->SetSettings(psSettings);
	}

	/** 
	 * Sets up a particle system using 3D mesh particles. The particles support lighting which is demonstrated via an
	 * addition of an orbiting point light. Once emitted the particles are evolved through the gravity evolver, ensuring
	 * they fall down. After which they collide with the ground plane by using the collider evolver.
	 */
	void setup3DParticleEffect(const Vector3& pos, const ParticleSystemAssets& assets)
	{
		// Create the particle system scene object and position/orient it
		HSceneObject particleSystemSO = SceneObject::Create("3D particles");
		particleSystemSO->SetPosition(pos);
		particleSystemSO->SetRotation(Quaternion(Degree(0), Degree(90), Degree(0)));

		// Add a particle system component
		HParticleSystem particleSystem = particleSystemSO->AddComponent<CParticleSystem>();

		// Set up the emitter
		SPtr<ParticleEmitter> emitter = bs_shared_ptr_new<ParticleEmitter>();

		// All newly spawned particles will have the size of 2cm
		emitter->SetInitialSize(0.02f);

		// 50 particles will be emitted per second
		emitter->SetEmissionRate(50.0f);

		// Particles will initially move at a rate of 1m/s
		emitter->SetInitialSpeed(1.0f);

		// Particles will live for exactly 5 seconds
		emitter->SetInitialLifetime(5.0f);

		// Set up a shape that determines the position and initial travel direction of newly spawned particles. In this
		// case we're using a cone shape.
		PARTICLE_CONE_SHAPE_DESC coneShape;

		// All particles will spawn at the narrow point in the cone (position doesn't vary)
		coneShape.Type = ParticleEmitterConeType::Base;

		// The particle travel direction will be in the 45 degrees spawned by the cone
		coneShape.Angle = Degree(45.0f);

		// Assign the shape to the emitter
		emitter->SetShape(ParticleEmitterConeShape::Create(coneShape));

		// Assign the emitter to the particle system
		particleSystem->SetEmitters({emitter});

		// Set up evolvers that will modify the particle systems over its lifetime
		Vector<SPtr<ParticleEvolver>> evolvers;

		// Set up an evolver at applies gravity to the particles. The gravity as set by the physics system is used, but
		// can be scaled as needed
		PARTICLE_GRAVITY_DESC gravityDesc;
		gravityDesc.Scale = 1.0f;

		// Create and add the gravity evolver
		SPtr<ParticleGravity> gravityEvolver = bs_shared_ptr_new<ParticleGravity>(gravityDesc);
		evolvers.push_back(gravityEvolver);

		// Set up an evolver that allows the particles to collide with the ground.
		PARTICLE_COLLISIONS_DESC collisionsDesc;

		// We use plane collisions but we could have also used world collisions (which are more expensive, but perform
		// general purpose collisions with all physical objects)
		collisionsDesc.Mode = ParticleCollisionMode::Plane;

		// Set up the particle radius used for collisions (2cm, same as visible size)
		collisionsDesc.Radius = 0.02f;

		// Create the collision evolver
		SPtr<ParticleCollisions> collisionEvolver = bs_shared_ptr_new<ParticleCollisions>(collisionsDesc);

		// Assign the plane the particles will collide with
		collisionEvolver->SetPlanes( { Plane(Vector3::UNIT_Y, 0.0f)});

		// Register the collision evolver
		evolvers.push_back(collisionEvolver);

		// Register all evolvers with the particle system
		particleSystem->SetEvolvers(evolvers);

		// Set up general particle system settings
		ParticleSystemSettings psSettings;

		// Specify that we want to render 3D meshes instead of billboards
		psSettings.RenderMode = ParticleRenderMode::Mesh;

		// Specify the mesh to use for particles
		psSettings.Mesh = assets.SphereMesh;

		// Set up a plain white diffuse material
		psSettings.Material = assets.Particles3DMat;

		// And actually apply the settings
		particleSystem->SetSettings(psSettings);

		// Set up an orbiting light
		//// Create the scene object, position and scale it
		HSceneObject lightSO = SceneObject::Create("Radial light");
		lightSO->SetPosition(pos - Vector3(0.0f, 0.8f, 0.0f));
		lightSO->SetScale(Vector3::ONE * 0.02f);

		//// Add the light component, emitting a red light
		HLight light = lightSO->AddComponent<CLight>();
		light->SetIntensity(30.0f);
		light->SetColor(Color::Red);
		light->SetUseAutoAttenuation(false);
		light->SetAttenuationRadius(20.0f);

		//// Add a sphere using an emissive material to represent the light
		HRenderable lightSphere = lightSO->AddComponent<CRenderable>();
		lightSphere->SetMesh(assets.SphereMesh);
		lightSphere->SetMaterial(assets.LightMat);

		//// Add a component that orbits the light at 1m of its original position
		lightSO->AddComponent<LightOrbit>(1.0f);
	}

	/** 
	 * Sets up a particle system that uses the GPU particle simulation. Particles are spawned on a surface of a sphere and
	 * a vector field is used for evolving the particles during their lifetime.
	 */
	void setupGPUParticleEffect(const Vector3& pos, const ParticleSystemAssets& assets)
	{
		// Create the particle system scene object and position/orient it
		HSceneObject particleSystemSO = SceneObject::Create("Vector field");
		particleSystemSO->SetPosition(pos);

		// Add a particle system component
		HParticleSystem particleSystem = particleSystemSO->AddComponent<CParticleSystem>();

		// Set up the emitter
		SPtr<ParticleEmitter> emitter = bs_shared_ptr_new<ParticleEmitter>();

		// All newly spawned particles will have the size of 1cm
		emitter->SetInitialSize(0.01f);

		// 400 particles will be emitted per second
		emitter->SetEmissionRate(400.0f);

		// No initial speed, we'll rely purely on the vector field force to move the particles
		emitter->SetInitialSpeed(0.0f);

		// Particles will live for exactly 5 seconds
		emitter->SetInitialLifetime(5.0f);

		// Set up a shape that determines the position of newly spawned particles. In this case spawn particles randomly
		// on a surface of a sphere.
		PARTICLE_SPHERE_SHAPE_DESC sphereShape;

		// Spawn on a sphere with radius of 30 cm
		sphereShape.Radius = 0.3f;

		// Assign the shape to the emitter
		emitter->SetShape(ParticleEmitterSphereShape::Create(sphereShape));

		// Assign the emitter to the particle system
		particleSystem->SetEmitters({emitter});

		// Set up general particle system settings
		ParticleSystemSettings psSettings;

		// Orient the particles towards the camera plane (standard for billboard particles)
		psSettings.Orientation = ParticleOrientation::ViewPlane;

		// But lock the Y orientation
		psSettings.OrientationLockY = true;

		// Sort by distance from camera so that transparency renders properly
		psSettings.SortMode = ParticleSortMode::Distance;

		// Use an emissive material to render the particles
		psSettings.Material = assets.LitParticleEmissiveMat;

		// Actually enable the GPU simulation
		psSettings.GpuSimulation = true;

		// Increase the maximum particle count since we'll be emitting them quickly
		psSettings.MaxParticles = 10000;

		// And actually apply the general settings
		particleSystem->SetSettings(psSettings);

		// Set up settings specific to the GPU simulation
		ParticleGpuSimulationSettings gpuSimSettings;

		// Set up a vector field. Use the vector field resource we imported earlier
		gpuSimSettings.VectorField.VectorField = assets.VectorField;

		// Increase the intensity of the forces in the vector field
		gpuSimSettings.VectorField.Intensity = 3.0f;

		// Setting this to zero ensures the vector field only applies forces, not velocities, to the particles
		gpuSimSettings.VectorField.Tightness = 0.0f;

		// And actually apply the GPU simulation settings
		particleSystem->SetGpuSimulationSettings(gpuSimSettings);
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

	// Set up the scene with an object to render and a camera
	setUpScene();

	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::Instance().RunMainLoop();

	// When done, clean up
	Application::ShutDown();

	return 0;
}
