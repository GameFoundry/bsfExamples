#include "BsFPSCamera.h"
#include "Math/BsVector3.h"
#include "Math/BsMath.h"
#include "Scene/BsSceneObject.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	/** Determines speed of camera rotation. */
	constexpr float ROTATION_SPEED = 3.0f;

	/** Determines range of movement for pitch rotation, in either direction. */
	constexpr Degree PITCH_RANGE = Degree(45.0f);

	FPSCamera::FPSCamera(const HSceneObject& parent)
		: Component(parent)
	{
		// Set a name for the component, so we can find it later if needed
		SetName("FPSCamera");

		// Get handles for key bindings. Actual keys attached to these bindings will be registered during app start-up.
		mHorizontalAxis = VirtualAxis("Horizontal");
		mVerticalAxis = VirtualAxis("Vertical");

		// Determine initial yaw and pitch
		Quaternion rotation = SO()->GetTransform().GetRotation();

		Radian pitch, yaw, roll;
		(void)rotation.ToEulerAngles(pitch, yaw, roll);

		mPitch = pitch;
		mYaw = yaw;

		ApplyAngles();
	}

	void FPSCamera::Update()
	{
		// If camera is rotating, apply new pitch/yaw rotation values depending on the amount of rotation from the
		// vertical/horizontal axes.
		mYaw += Degree(gVirtualInput().GetAxisValue(mHorizontalAxis) * ROTATION_SPEED);
		mPitch += Degree(gVirtualInput().GetAxisValue(mVerticalAxis) * ROTATION_SPEED);

		ApplyAngles();
	}

	void FPSCamera::ApplyAngles()
	{
		mYaw.Wrap();
		mPitch.Wrap();

		const Degree pitchMax = PITCH_RANGE;
		const Degree pitchMin = Degree(360.0f) - PITCH_RANGE;

		if(mPitch > pitchMax && mPitch < pitchMin)
		{
			if((mPitch - pitchMax) > (pitchMin - mPitch))
				mPitch = pitchMin;
			else
				mPitch = pitchMax;
		}

		Quaternion yRot(Vector3::UNIT_Y, Radian(mYaw));
		Quaternion xRot(Vector3::UNIT_X, Radian(mPitch));

		if(!mCharacterSO)
		{
			Quaternion camRot = yRot * xRot;
			camRot.Normalize();

			SO()->SetRotation(camRot);
		}
		else
		{
			mCharacterSO->SetRotation(yRot);
			SO()->SetRotation(xRot);
		}
	}

} // namespace bs
