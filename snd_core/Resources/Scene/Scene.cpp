#include "snd_core/Resources/Scene/Scene.hpp"

auto Scene::push_material(MaterialData&& material) -> void {
    m_data.materials.emplace_back(material);
}
auto Scene::push_texture(Texture&& texture) -> void {
    m_data.textures.emplace_back(texture);
}
auto Scene::push_image(ImageData&& image) -> void {
    m_data.images.emplace_back(image);
}
auto Scene::push_primitive(std::vector<Vertex>&& vertices) -> void {
}
auto Scene::push_primitive_instance(PrimitiveInstance&& instance) -> void {
}
auto Scene::push_model(ModelRange model_range) -> void {
}
auto Scene::push_sampler(SamplerData&& sampler_data) -> void {
}

auto Scene::new_model_range() -> ModelRange {
    auto model_range = ModelRange {
        .primitive_instances =
            IndexSpan {
                .index = static_cast<uint32_t>(m_data.primitive_instances.size()),
                .length = 0,
            },
        .primitives =
            IndexSpan {
                .index = static_cast<uint32_t>(m_data.primitives.size()),
                .length = 0,
            },
        .materials =
            IndexSpan {
                .index = static_cast<uint32_t>(m_data.materials.size()),
                .length = 0,
            },
        .textures =
            IndexSpan {
                .index = static_cast<uint32_t>(m_data.textures.size()),
                .length = 0,
            },
        .samplers =
            IndexSpan {
                .index = static_cast<uint32_t>(m_data.samplers.size()),
                .length = 0,
            },
        .vertices =
            IndexSpan {
                .index = static_cast<uint32_t>(m_data.vertices.size()),
                .length = 0,
            },
        .indices =
            IndexSpan {
                .index = static_cast<uint32_t>(m_data.indices.size()),
                .length = 0,
            },
        .image_data = IndexSpan {
            .index = static_cast<uint32_t>(m_data.image_data.size()),
            .length = 0,
        },

    };
}

auto Scene::get_texture_index_with_white_fallback(int32_t index) -> uint32_t {
    if (index == -1) {
        return 0;
    }

    return index + TEXTURES_OFFSET + m_data.last_element_offsets.textures;
}

auto Scene::get_texture_index_with_black_fallback(int32_t index) -> uint32_t {
    if (index == -1) {
        return 1;
    }

    return index + TEXTURES_OFFSET + m_data.last_element_offsets.textures;
}

auto Scene::get_material_index_with_default_fallback(SceneData& data, int32_t index) -> uint32_t {
    if (index == -1) {
        return 0;
    }
    return static_cast<uint32_t>(index);
}
