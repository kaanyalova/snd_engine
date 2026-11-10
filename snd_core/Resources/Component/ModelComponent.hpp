#pragma once
#include <vulkan/vulkan_core.h>

#include "../Scene/SceneData.hpp"
#include "snd_core/Resources/Component/Component.hpp"

class ModelComponent : public Component {
  public:
    ModelComponent(ModelRange model_range);

    auto get_model_range() const -> const ModelRange& { return m_model_range; }
    // auto set_transform(const glm::mat4x4& transform) -> void { /* TODO */ }
    auto set_material_index(uint32_t index) -> void { /* TODO */ }

  private:
    ModelRange m_model_range;
    uint64_t m_scene_transform_index;
};
