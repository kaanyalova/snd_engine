#pragma once
#include <vulkan/vulkan_core.h>

#include "../Scene/SceneData.hpp"
#include "snd_core/Resources/Component/Component.hpp"

class ModelComponent : public Component {
  public:
    auto get_model_range() const -> const ModelRange& { return model_range; }

  private:
    ModelRange model_range;
    glm::mat4x4 transform;
};