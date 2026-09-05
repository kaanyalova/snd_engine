#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "SceneData.hpp"
#include "snd_core/Image/Image.hpp"
#include "snd_core/Renderer/Vulkan/VulkanScene.hpp"
#include "snd_core/Resources/Scene/SceneNode.hpp"

enum class SceneLoadState : uint8_t {
    Unloaded,
    LoadedOnCpu,
    LoadedOnGpu,
};

class Scene {
  public:
    Scene() = default;

    auto get_data() -> const SceneData& { return m_data; }

    auto push_material(MaterialData&& material) -> void;
    auto push_texture(Texture&& texture) -> void;
    auto push_image(ImageData&& image) -> void;
    auto push_primitive(std::vector<Vertex>&& vertices) -> void;
    auto push_primitive_instance(PrimitiveInstance&& instance) -> void;
    auto push_model(ModelRange model_range) -> void;
    auto push_sampler(SamplerData&& sampler_data) -> void;

    auto push_scene_data(SceneData&& other) -> void;
    auto push_scene(Scene&& other) -> void;

    auto get_root_node() -> SceneNode&;

    /**
     * Create a ModelRange that has the correct offsets for the data inside of it
     * so it can be filled with data by the loaders, by using the push_ functions
     * @return A new ModelRange that has the correct offsets for each element
     */
    auto new_model_range() -> ModelRange;

    auto get_texture_index_with_white_fallback(int32_t index) -> uint32_t;
    auto get_texture_index_with_black_fallback(int32_t index) -> uint32_t;
    auto get_material_index_with_default_fallback(SceneData& data, int32_t index) -> uint32_t;

  private:
    SceneNode root = SceneNode();
    PushOffsets m_push_offsets;

    // the data is arranged linearly for each field
    SceneData m_data;

    std::optional<VulkanScene> m_vulkan_scene = std::nullopt;
    SceneLoadState m_scene_load_state = SceneLoadState::Unloaded;

    static constexpr uint32_t TEXTURES_OFFSET = 2;
    static constexpr uint32_t MATERIALS_OFFSET = 1;
};
