#include "snd_core/Resources/Scene/Scene.hpp"

auto Scene::push_scene_data(const SceneData& other) -> void {
    ModelRange model_range = {};

    model_range.primitive_instances.index = m_data.primitive_instances.size();
    model_range.primitive_instances.length = other.primitive_instances.size();
    m_data.primitive_instances.append_range(other.primitive_instances);

    model_range.primitives.index = m_data.primitives.size();
    model_range.primitives.length = other.primitives.size();
    m_data.primitives.append_range(other.primitives);

    model_range.materials.index = m_data.materials.size();
    model_range.materials.length = other.materials.size();
    m_data.materials.append_range(other.materials);

    model_range.textures.index = m_data.textures.size();
    model_range.textures.length = other.textures.size();
    m_data.textures.append_range(other.textures);

    model_range.samplers.index = m_data.samplers.size();
    model_range.samplers.length = other.samplers.size();
    m_data.samplers.append_range(other.samplers);

    model_range.vertices.index = m_data.vertices.size();
    model_range.vertices.length = other.vertices.size();
    m_data.vertices.append_range(other.vertices);

    model_range.indices.index = m_data.indices.size();
    model_range.indices.length = other.indices.size();
    m_data.indices.append_range(other.indices);

    model_range.image_data.index = m_data.image_data.size();
    model_range.image_data.length = other.image_data.size();
    m_data.image_data.append_range(other.image_data);

    m_models.emplace_back(model_range);
}

auto Scene::get_texture_index_with_white_fallback(int32_t index) -> uint32_t {
    if (index == -1) {
        return 0;
    }

    return index + TEXTURES_OFFSET;
}

auto Scene::get_texture_index_with_black_fallback(int32_t index) -> uint32_t {
    if (index == -1) {
        return 1;
    }

    return index + TEXTURES_OFFSET;
}

auto Scene::get_material_index_with_default_fallback(SceneData& data, int32_t index) -> uint32_t {
    if (index == -1) {
        return 0;
    }
    return static_cast<uint32_t>(index);
}
auto Scene::mark_loaded_on_gpu(VulkanScene* vulkan_scene) -> void {
    m_scene_load_state = SceneLoadState::LoadedOnGpu;
    m_vulkan_scene = vulkan_scene;
    m_data = {};
}
