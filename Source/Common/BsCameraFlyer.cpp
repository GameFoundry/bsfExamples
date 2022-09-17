#include "BsCameraFlyer.h"
#include "Math/BsVector3.h"
#include "Utility/BsTime.h"
#include "Math/BsMath.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCCamera.h"
#include "Platform/BsCursor.h"

namespace bs
{
	const float CameraFlyer::START_SPEED = 40.0f;
	const float CameraFlyer::TOP_SPEED = 130.0f;
	const float CameraFlyer::ACCELERATION = 10.0f;
	const float CameraFlyer::FAST_MODE_MULTIPLIER = 2.0f;
	const float CameraFlyer::ROTATION_SPEED = 3.0f;

	/** Wraps an angle so it always stays in [0, 360) range. */
	Degree wrapAngle(Degree angle)
	{
		if (angle.ValueDegrees() < -360.0f)
			angle += Degree(360.0f);

		if (angle.ValueDegrees() > 360.0f)
			angle -= Degree(360.0f);

		return angle;
	}

	CameraFlyer::CameraFlyer(const HSceneObject& parent)
		:Component(parent)
	{
		// Set a name for the component, so we can find it later if needed
		SetName("CameraFlyer");

		// Get handles for key bindings. Actual keys attached to these bindings will be registered during app start-up.
		mMoveForward = VirtualButton("Forward");
		mMoveBack = VirtualButton("Back");
		mMoveLeft = VirtualButton("Left");
		mMoveRight = VirtualButton("Right");
		mFastMove = VirtualButton("FastMove");
		mRotateCam = VirtualButton("RotateCam");
		mHorizontalAxis = VirtualAxis("Horizontal");
		mVerticalAxis = VirtualAxis("Vertical");
	}

	void CameraFlyer::Update()
	{
		// Check if any movement or rotation keys are being held
		bool goingForward = gVirtualInput().IsButtonHeld(mMoveForward);
		bool goingBack = gVirtualInput().IsButtonHeld(mMoveBack);
		bool goingLeft = gVirtualInput().IsButtonHeld(mMoveLeft);
		bool goingRight = gVirtualInput().IsButtonHeld(mMoveRight);
		bool fastMove = gVirtualInput().IsButtonHeld(mFastMove);
		bool camRotating = gVirtualInput().IsButtonHeld(mRotateCam);

		// If switch to or from rotation mode, hide or show the cursor
		if (camRotating != mLastButtonState)
		{
			if (camRotating)
				Cursor::Instance().Hide();
			else
				Cursor::Instance().Show();

			mLastButtonState = camRotating;
		}

		// If camera is rotating, apply new pitch/yaw rotation values depending on the amount of rotation from the
		// vertical/horizontal axes.
		float frameDelta = gTime().GetFrameDelta();
		if (camRotating)
		{
			mYaw += Degree(gVirtualInput().GetAxisValue(mHorizontalAxis) * ROTATION_SPEED);
			mPitch += Degree(gVirtualInput().GetAxisValue(mVerticalAxis) * ROTATION_SPEED);

			mYaw = wrapAngle(mYaw);
			mPitch = wrapAngle(mPitch);

			Quaternion yRot;
			yRot.FromAxisAngle(Vector3::UNIT_Y, Radian(mYaw));

			Quaternion xRot;
			xRot.FromAxisAngle(Vector3::UNIT_X, Radian(mPitch));

			Quaternion camRot = yRot * xRot;
			camRot.Normalize();

			SO()->SetRotation(camRot);
		}

		const Transform& tfrm = SO()->GetTransform();

		// If the movement button is pressed, determine direction to move in
		Vector3 direction = Vector3::ZERO;
		if (goingForward) direction += tfrm.GetForward();
		if (goingBack) direction -= tfrm.GetForward();
		if (goingRight) direction += tfrm.GetRight();
		if (goingLeft) direction -= tfrm.GetRight();

		// If a direction is chosen, normalize it to determine final direction.
		if (direction.SquaredLength() != 0)
		{
			direction.Normalize();

			// Apply fast move multiplier if the fast move button is held.
			float multiplier = 1.0f;
			if (fastMove)
				multiplier = FAST_MODE_MULTIPLIER;

			// Calculate current speed of the camera
			mCurrentSpeed = Math::Clamp(mCurrentSpeed + ACCELERATION * frameDelta, START_SPEED, TOP_SPEED);
			mCurrentSpeed *= multiplier;
		}
		else
		{
			mCurrentSpeed = 0.0f;
		}

		// If the current speed isn't too small, move the camera in the wanted direction
		float tooSmall = std::numeric_limits<float>::epsilon();
		if (mCurrentSpeed > tooSmall)
		{
			Vector3 velocity = direction * mCurrentSpeed;
			SO()->Move(velocity * frameDelta);
		}
	}
}