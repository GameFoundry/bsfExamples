#include "Scene/BsComponent.h"

namespace bs
{

// Spinning asteroid component.
class Random;
class CSpinner : public Component
{
  Vector3 rotAxis;
  float speed;
public:
  CSpinner(const HSceneObject& parent, Random* rand);
  void fixedUpdate() override;
};

class COrbiter : public Component
{
public:
  float speed;
  COrbiter(const HSceneObject& parent, Random* rand);
  void fixedUpdate() override;
};


} // namespace bs
