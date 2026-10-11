#pragma once

#include <optional>
#include <vector>

#include "../../Renderer/Vulkan/Resources/VulkanScene.hpp"
#include "SceneData.hpp"
#include "glm/fwd.hpp"
#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"
#include "snd_core/Resources/Scene/SceneNode.hpp"

enum class SceneLoadState : uint8_t {
    Unloaded,
    LoadedOnCpu,
    LoadedOnGpu,
};

// - flatten all model ranges inside the modelcomponents into primitive instances
// - make them have indices to the scene transform buffer, so they know which node transform should they have
// - group them by model->primitive so i can draw duplicate primitives in a single command of the indirect whatever
// - build indirect draw ranges from the primitive instances
// -

struct ScenePrimitiveInstance {
    uint32_t primitive_index;
    glm::mat4x4 transform;
    uint32_t node_transform_index;
};

struct BakedScene {
    std::vector<IndexSpan> primitive_draw_ranges;
    std::vector<ScenePrimitiveInstance> baked_primitives;
    std::vector<glm::mat4x4> scene_node_transforms;
};

/*
  for (i, primitive) in primitive_ranges:
    struct VkDrawIndexedIndirectCommand {
        uint32_t    indexCount; // scene.primitives[i].indices.count
        uint32_t    instanceCount; // primitive_range.count
        uint32_t    firstIndex;   // scene.primitives[i].indices.index
        int32_t     vertexOffset; // 0
        uint32_t    firstInstance; // primitive_range.index
};

*/

class Scene {
  public:
    Scene(VulkanDevice& device);

    auto get_data() const -> const SceneData& { return m_data; }
    auto push_scene_data(const SceneData& other) -> void;
    auto get_root_node() const -> const SceneNode& { return m_root; }
    auto get_models() const -> const std::vector<ModelRange>& { return m_models; }

    static auto get_texture_index_with_white_fallback(int32_t index) -> uint32_t;
    static auto get_texture_index_with_black_fallback(int32_t index) -> uint32_t;
    static auto get_material_index_with_default_fallback(SceneData& data, int32_t index) -> uint32_t;

    auto mark_loaded_on_gpu(VulkanScene* vulkan_scene) -> void;

  private:
    SceneNode m_root = SceneNode();

    // the data is arranged linearly for each field
    SceneData m_data;
    // the ranges for each individual model thats loaded
    std::vector<ModelRange> m_models;

    VulkanScene* m_vulkan_scene = nullptr;
    SceneLoadState m_scene_load_state = SceneLoadState::Unloaded;

    // the first two textures in a scene is black then white
    static constexpr uint32_t TEXTURES_OFFSET = 2;
    // the first material is the default material for the models that don't have them
    static constexpr uint32_t MATERIALS_OFFSET = 1;
};
