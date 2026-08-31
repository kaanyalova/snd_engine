#pragma once
#include "../Scene/SceneData.hpp"
#include "snd_core/Resources/Component/Component.hpp"

class ModelComponent : public Component {
  public:
  private:
    ModelRange model_range;
    glm::mat4x4 transform;
};