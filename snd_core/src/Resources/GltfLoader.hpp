#pragma once
#include <tiny_gltf_v3.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glm/ext/matrix_float4x4.hpp>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../Utils/HashUtils.hpp"
#include "SceneData.hpp"

class GltfLoader {
  public:
    GltfLoader(std::string base_directory);

    auto load_into_scene_with_opts(
        SceneData& scene_data, std::span<const uint8_t> bytes, const tg3_parse_options& opts
    ) -> void;

    auto load_into_scene(SceneData& scene, std::span<const uint8_t> bytes) -> void;

    auto load_into_scene_from_path(SceneData& scene, const std::filesystem::path& path) -> void;
    auto load_into_scene_from_path_with_opts(
        SceneData& scene, const std::filesystem::path& path, const tg3_parse_options& opts
    ) -> void;

  private:
    template <typename T>
    struct AccessorView {
        uint32_t byte_stride;
        uint32_t length;
        const T* pointer;

        [[nodiscard]] auto get(size_t index) const -> const T* {
            auto byte_pointer = reinterpret_cast<const uint8_t*>(pointer);
            auto item_byte_pointer = byte_pointer + index * byte_stride;
            return reinterpret_cast<const T*>(item_byte_pointer);
        }

        [[nodiscard]] auto hash() const -> size_t {
            size_t hash = 0;
            HashUtils::hash_combine(hash, byte_stride);
            HashUtils::hash_combine(hash, length);
            HashUtils::hash_combine(hash, reinterpret_cast<uintptr_t>(pointer));
            return hash;
        }
    };

    // get a pointer to the actual data inside an tg3_accessor, assuming what is inside
    // the accessor is of type T
    template <typename T>
    auto get_accessor_view(const tg3_accessor* accessor, const tg3_model& model)
        -> AccessorView<T> {
        uint32_t bytes_stride = 0;

        size_t accessor_offset = accessor->byte_offset;

        // make sure the accessor points to a buffer
        assert(accessor->buffer_view != -1);

        const tg3_buffer_view& buffer_view = model.buffer_views[accessor->buffer_view];
        size_t buffer_view_offset = buffer_view.byte_offset;
        bytes_stride = buffer_view.byte_stride;

        if (bytes_stride == 0) {
            bytes_stride = sizeof(T);
        }

        const tg3_buffer& buffer = model.buffers[buffer_view.buffer];
        const uint8_t* raw_pointer = buffer.data.data + buffer_view_offset + accessor_offset;

        return AccessorView<T> {
            .byte_stride = bytes_stride,
            .length = static_cast<uint32_t>(accessor->count),
            .pointer = reinterpret_cast<const T*>(raw_pointer),
        };
    }

    struct PrimitiveInstanceKey {
        AccessorView<glm::vec3> positions;
        AccessorView<glm::vec2> texture_coords;
        AccessorView<glm::vec3> normals;
    };

    struct PrimitiveInstanceKeyHasher {
        auto operator()(const PrimitiveInstanceKey& key) -> size_t const {
            size_t hash = 0;
            HashUtils::hash_combine(hash, key.positions.hash());
            HashUtils::hash_combine(hash, key.texture_coords.hash());
            HashUtils::hash_combine(hash, key.normals.hash());
            return hash;
        }
    };
    const static uint32_t TEXTURES_OFFSET = 2;
    const static uint32_t MATERIALS_OFFSET = 1;

    using GltfModelCache =
        std::unordered_map<PrimitiveInstanceKey, uint32_t, PrimitiveInstanceKeyHasher>;

    auto traverse_node(
        SceneData& data,
        ModelRange& model_range,
        GltfModelCache& cache,
        const tg3_model& gltf_model,
        const tg3_node& gltf_node,
        const glm::mat4x4& parent_transform
    ) -> void;

    auto unwrap_error_stack() -> void;
    auto get_severity_string(tg3_severity severity) -> std::string;
    auto get_node_transform(const tg3_node& node) -> glm::mat4x4;
    auto push_primitive(
        const tg3_primitive& primitive,
        glm::mat4 world_transform,
        int32_t primitive_index,
        SceneData& data,
        const tg3_model& model,
        ModelRange& model_range,
        GltfModelCache& cache
    ) -> void;

    auto load_materials(SceneData& data, ModelRange& model_range, const tg3_model& model) -> void;
    auto load_textures(SceneData& data, ModelRange& model_range, const tg3_model& model) -> void;
    auto load_samplers(SceneData& data, ModelRange& model_range, const tg3_model& model) -> void;

    auto new_color_texture(SceneData& data) -> void;

    auto get_texture_index_with_white_fallback(SceneData& data, int32_t index) -> uint32_t;
    auto get_texture_index_with_black_fallback(SceneData& data, int32_t index) -> uint32_t;

    auto get_material_index_with_default_fallback(SceneData& data, int32_t index) -> uint32_t;

    auto next_model_range(SceneData& data) -> ModelRange;

    auto tg3_str_to_string_view(const tg3_str& str) -> std::string_view;

    [[nodiscard]] auto weave_vertex_data_from_accessors(
        const AccessorView<glm::vec3> positions,
        const AccessorView<glm::vec2> texture_coords,
        const AccessorView<glm::vec3> normals,
        const tg3_model& model
    ) -> std::vector<Vertex>;

    std::string m_base_directory;
    tg3_error_stack m_error_stack;
    tg3_parse_options m_default_parse_options;
};