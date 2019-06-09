#include "./CSpinner.h"
#include "Scene/BsSceneObject.h"
#include "Math/BsRandom.h"

namespace bs
{

CSpinner::CSpinner(const HSceneObject& parent, Random* rand) :
  Component(parent),
  rotAxis(rand->getUnitVector()),
  speed(rand->getUNorm() * 0.005 + 0.005) {}

void CSpinner::fixedUpdate() {
  SO()->rotate(rotAxis, Radian(speed));
}

COrbiter::COrbiter(const HSceneObject& parent, Random* rand) :
  Component(parent),
  speed(rand->getUNorm() * 0.0005 + 0.0005) {}

void COrbiter::fixedUpdate() {
  // SO()->rotate(Vector3::UNIT_Y, Radian(0.001));
  // Q()
  // auto q = SO()->getTransform().getRotation();
  Quaternion rot;
  rot.fromEulerAngles(Radian(0), Radian(speed), Radian(0));
  auto pos = SO()->getTransform().getPosition();
  // std::cout << "POS " << pos.x << " " << pos.y << std::endl;
  pos = rot.rotate(pos);
  SO()->setWorldPosition(pos);
}

}
