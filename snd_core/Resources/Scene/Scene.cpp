#include "snd_core/Resources/Scene/Scene.hpp"

#include <ranges>

Scene::Scene() {
}

auto Scene::push_scene_data(const SceneData& other) -> void {
    ModelRange model_range = {};

    model_range.primitive_instances.index = m_data.primitive_instances.size();
    model_range.primitive_instances.length = other.primitive_instances.size();

    model_range.primitives.index = m_data.primitives.size();
    model_range.primitives.length = other.primitives.size();

    // fix others local primitive_instances to use the actual scene data
    for (const auto& instance : other.primitive_instances) {
        m_data.primitive_instances.emplace_back(
            PrimitiveInstance {
                .transform = instance.transform,
                .primitive_index = instance.primitive_index + model_range.primitives.index,
            }
        );
    }

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

    model_range.images.index = m_data.images.size();
    model_range.images.length = other.images.size();
    m_data.images.append_range(other.images);

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
auto Scene::mark_loaded_on_gpu() -> void {
    m_scene_load_state = SceneLoadState::LoadedOnGpu;
    m_data = {};
}

auto Scene::bake_scene() -> void {
    m_baked_scene = BakedScene {};

    traverse_and_bake_scene_node_transforms(&m_root_node);
    traverse_and_bake_scene_primitive_instances(&m_root_node);
    sort_primitive_instances_and_build_ranges();
}

auto Scene::traverse_and_bake_scene_primitive_instances(SceneNode* node) -> void {
    for (const auto& model : node->get_model_components()) {
        const ModelRange& model_range = model.get_model_range();

        for (size_t i = 0; i < model_range.primitive_instances.length; i++) {
            size_t instance_index = model_range.primitive_instances.index + i;
            const PrimitiveInstance& instance = m_data.primitive_instances[instance_index];

            auto baked_instance = BakedPrimitiveInstance {
                .primitive_index = instance.primitive_index,
                .node_transform_index = node->get_baked_transform_index(),
                .transform = instance.transform,
            };

            m_baked_scene->primitive_instances.emplace_back(baked_instance);
        }
    }

    for (auto& child : node->get_mutable_children()) {
        traverse_and_bake_scene_primitive_instances(child.get());
    }
}

auto Scene::sort_primitive_instances_and_build_ranges() -> void {
    std::ranges::sort(m_baked_scene->primitive_instances, [](BakedPrimitiveInstance& lhs, BakedPrimitiveInstance& rhs) {
        return lhs.primitive_index < rhs.primitive_index;
    });

    auto chunks = std::views::chunk_by(
        m_baked_scene->primitive_instances, [](BakedPrimitiveInstance& lhs, BakedPrimitiveInstance& rhs) {
            return lhs.primitive_index == rhs.primitive_index;
        }
    );

    for (const auto& chunk : chunks) {
        auto span = IndexSpan {
            .index = static_cast<uint32_t>(chunk.begin() - m_baked_scene->primitive_instances.begin()),
            .length = static_cast<uint32_t>(chunk.size()),
        };

        m_baked_scene->primitive_draw_ranges.emplace_back(span);
    }
}

auto Scene::traverse_and_bake_scene_node_transforms(SceneNode* node) -> void {
    if (node->has_parent()) {
        node->set_transform(node->get_parent()->get_transform() * node->get_transform());
    }

    if (m_baked_scene.has_value()) {
        node->set_baked_transform_index(m_baked_scene->scene_node_transforms.size());
        m_baked_scene->scene_node_transforms.emplace_back(node->get_transform());
    }

    for (auto& child : node->get_mutable_children()) {
        traverse_and_bake_scene_node_transforms(child.get());
    }
}

auto Scene::get_stats() -> std::string {
    std::string result;
    result += m_data.get_scene_stats();

    if (m_baked_scene.has_value()) {
        result += m_baked_scene->get_stats();
    } else {
        result += "Scene is not baked\n";
    }

    return result;
}
