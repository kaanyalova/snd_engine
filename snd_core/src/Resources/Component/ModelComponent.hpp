#pragma once
#include "../SceneData.hpp"
#include "Component.hpp"

class ModelComponent : public Component {
  public:
  private:
    ModelRange model_range;
    glm::mat4x4 transform;
};