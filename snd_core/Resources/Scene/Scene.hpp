#pragma once

#include <optional>
#include <vector>

#include "../../Renderer/Vulkan/Resources/VulkanScene.hpp"
#include "SceneData.hpp"
#include "snd_core/Renderer/Vulkan/VulkanDevice.hpp"
#include "snd_core/Resources/Scene/SceneNode.hpp"

enum class SceneLoadState : uint8_t {
    Unloaded,
    LoadedOnCpu,
    LoadedOnGpu,
};

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
