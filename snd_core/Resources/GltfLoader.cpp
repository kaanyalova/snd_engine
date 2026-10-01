
#define TINYGLTF3_IMPLEMENTATION
#include "snd_core/Resources/GltfLoader.hpp"

#include <tiny_gltf_v3.h>

#include <cassert>
#include <format>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "snd_core/Image/KtxImage.hpp"
#include "snd_core/Image/StbImage.hpp"
#include "snd_core/Resources/Scene/Scene.hpp"
#include "snd_core/Utils/FileUtils.hpp"

GltfLoader::GltfLoader(std::string base_directory) : m_base_directory(std::move(base_directory)) {
    tg3_error_stack_init(&m_error_stack);
    tg3_parse_options_init(&m_default_parse_options);
}

auto GltfLoader::load_into_scene_from_path(SceneData& scene, const std::filesystem::path& path) -> void {
    load_into_scene_from_path_with_opts(scene, path, m_default_parse_options);
}

auto GltfLoader::load_into_scene_from_path_with_opts(
    SceneData& scene, const std::filesystem::path& path, const tg3_parse_options& opts
) -> void {
    std::vector<uint8_t> file = FileUtils::read_file(path);
    load_into_scene_with_opts(scene, file, opts);
}

auto GltfLoader::load_into_scene(SceneData& scene, std::span<const uint8_t> bytes) -> void {
    load_into_scene_with_opts(scene, bytes, m_default_parse_options);
}

auto GltfLoader::load_into_scene_with_opts(
    SceneData& scene, std::span<const uint8_t> bytes, const tg3_parse_options& opts
) -> void {
    tg3_model model;
    tg3_parse_auto(
        &model, &m_error_stack, bytes.data(), bytes.size(), m_base_directory.c_str(), m_base_directory.length(), &opts
    );
    unwrap_error_stack();

    GltfModelCache cache = {};

    for (size_t i = 0; i < model.scenes_count; i++) {
        const tg3_scene& tg3_scene = model.scenes[i];

        for (size_t i = 0; i < tg3_scene.nodes_count; i++) {
            const tg3_node& node = model.nodes[tg3_scene.nodes[i]];
            traverse_node(scene, cache, model, node, glm::mat4x4(1.0f));
        }
    }

    load_images(scene, model);
    load_materials(scene, model);
    load_textures(scene, model);
    load_samplers(scene, model);
};

auto GltfLoader::traverse_node(
    SceneData& scene,
    GltfModelCache& cache,
    const tg3_model& gltf_model,
    const tg3_node& gltf_node,
    const glm::mat4x4& parent_transform
) -> void {
    const glm::mat4x4 local = get_node_transform(gltf_node);
    const glm::mat4x4 world = parent_transform * local;

    // process the mesh thats attached to the node
    uint32_t mesh_index = gltf_node.mesh;
    if (mesh_index != -1) {
        const tg3_mesh& mesh = gltf_model.meshes[mesh_index];

        for (size_t i = 0; i < mesh.primitives_count; i++) {
            const tg3_primitive& primitive = mesh.primitives[i];
            push_primitive(primitive, world, i, scene, gltf_model, cache);

            // push_primitive(m_c, SceneData &data, const tg3_model &model, const tg3_primitive
            // &primitive, ModelRange &model_range, int32_t primitive_index, glm::mat4
            // world_transform)
        }
    }

    for (size_t i = 0; i < gltf_node.children_count; i++) {
        uint32_t node_index = gltf_node.children[i];
        const tg3_node& child_node = gltf_model.nodes[node_index];
        traverse_node(scene, cache, gltf_model, child_node, world);
    }
}

auto GltfLoader::unwrap_error_stack() -> void {
    if (m_error_stack.has_error) {
        std::string error_messages = "";

        for (size_t i = 0; i < m_error_stack.count; i++) {
            const tg3_error_entry& error = m_error_stack.entries[i];
            std::string error_message = std::format("{}: {}\n", get_severity_string(error.severity), error.message);
        }

        throw std::runtime_error(error_messages);
    }
}

auto GltfLoader::get_severity_string(tg3_severity severity) -> std::string {
    switch (severity) {
        case TG3_SEVERITY_INFO:
            return "INFO";
        case TG3_SEVERITY_WARNING:
            return "WARNING";
        case TG3_SEVERITY_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

auto GltfLoader::get_node_transform(const tg3_node& node) -> glm::mat4x4 {
    auto transform = glm::mat4x4(1.0f);

    // its either a 4x4 in the node.transform
    if (node.has_matrix) {
        transform = glm::make_mat4(node.matrix);
    } else {
        // or stored in translation, rotation, scale
        const glm::vec3 translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
        transform = glm::translate(transform, translation);

        const glm::quat rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
        transform *= glm::mat4_cast(rotation);

        const glm::vec3 scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
        transform = glm::scale(transform, scale);
    }

    return transform;
}

/**
 * @brief Push a primitives and primitive instances into the scene
 * PrimitiveInstances are always pushed, but the data is cached using the GltfModelCache
 */
auto GltfLoader::push_primitive(
    const tg3_primitive& primitive,
    glm::mat4 world_transform,
    int32_t primitive_index,
    SceneData& data,
    const tg3_model& model,
    GltfModelCache& cache
) -> void {
    if (primitive.mode != TG3_MODE_TRIANGLES) {
        std::println(
            "primitive mode was not triangles for the primitive at index {}, setting it to "
            "triangles anyway",
            primitive_index
        );
    }

    // map the accessors, we only care about target[0] for now
    // the others are for this
    // https://github.com/KhronosGroup/glTF-Tutorials/blob/main/gltfTutorial/gltfTutorial_017_SimpleMorphTarget.md
    const tg3_str_int_pair* attributes = primitive.attributes;

    std::optional<AccessorView<glm::vec3>> position_accessor = std::nullopt;
    std::optional<AccessorView<glm::vec3>> normal_accessor = std::nullopt;
    std::optional<AccessorView<glm::vec2>> texture_coord_accessor = std::nullopt;
    std::optional<AccessorView<glm::vec3>> tangent_accessor = std::nullopt;

    for (size_t i = 0; i < primitive.attributes_count; i++) {
        tg3_str attribute_key_c_str = attributes[i].key;
        std::string_view attribute_key = std::string_view(attribute_key_c_str.data, attribute_key_c_str.len);
        int32_t attribute_value = attributes[i].value;

        if (attribute_key == "POSITION") {
            position_accessor = get_accessor_view<glm::vec3>(&model.accessors[attribute_value], model);
        }

        else if (attribute_key == "NORMAL") {
            normal_accessor = get_accessor_view<glm::vec3>(&model.accessors[attribute_value], model);
        }

        else if (attribute_key == "TEXCOORD_0") {
            texture_coord_accessor = get_accessor_view<glm::vec2>(&model.accessors[attribute_value], model);
        }

        else if (attribute_key == "TANGENT") {
            tangent_accessor = get_accessor_view<glm::vec3>(&model.accessors[attribute_value], model);
        }
    }

    bool does_accessors_exist =
        position_accessor.has_value() && normal_accessor.has_value() && texture_coord_accessor.has_value();

    // check the cache here
    if (!does_accessors_exist) {
        throw std::runtime_error(
            std::format("One of the accessors for the primitive at index {} doesn't exist", primitive_index)
        );
    }

    // always push the instance
    data.primitive_instances.emplace_back(
        PrimitiveInstance {
            .transform = world_transform,
            .primitive_index = static_cast<uint32_t>(data.primitives.size()),
        }
    );

    // push the data if it isn't in the cache
    std::vector<Vertex> vertices = weave_vertex_data_from_accessors(
        position_accessor.value(), texture_coord_accessor.value(), normal_accessor.value(), model
    );

    data.vertices.append_range(vertices);
    data.primitives.emplace_back(
        Primitive {
            .material_index = get_material_index_with_default_fallback(data, primitive.material),
            .vertex_span = IndexSpan {
                .index = static_cast<uint32_t>(data.vertices.size() - vertices.size()),
                .length = static_cast<uint32_t>(vertices.size()),
            },
        }
    );
}
auto GltfLoader::load_samplers(SceneData& scene, const tg3_model& model) -> void {
    for (size_t i = 0; i < model.samplers_count; i++) {
        const tg3_sampler& sampler = model.samplers[i];

        auto sampler_data = SamplerData {
            .min_filter = sampler.min_filter,
            .mag_filter = sampler.mag_filter,
            .wrap_s = sampler.wrap_s,
            .wrap_t = sampler.wrap_t,
        };

        scene.samplers.emplace_back(sampler_data);
    }

    // scene.last_element_offsets.samplers += model.samplers_count;
    // model_range.samplers.length += model.samplers_count;
}

auto GltfLoader::load_materials(SceneData& scene, const tg3_model& model) -> void {
    for (size_t i = 0; i < model.materials_count; i++) {
        const tg3_material& material = model.materials[i];

        auto material_data = MaterialData {
            .base_color_texture_index =
                Scene::get_texture_index_with_white_fallback(material.pbr_metallic_roughness.base_color_texture.index),
            .base_color_texture_factor = glm::vec4(
                material.pbr_metallic_roughness.base_color_factor[0],
                material.pbr_metallic_roughness.base_color_factor[1],
                material.pbr_metallic_roughness.base_color_factor[2],
                material.pbr_metallic_roughness.base_color_factor[3]
            ),
            .metallic_roughness_texture_index = Scene::get_texture_index_with_white_fallback(
                material.pbr_metallic_roughness.metallic_roughness_texture.index
            ),
            .roughness_factor = static_cast<float>(material.pbr_metallic_roughness.roughness_factor),
            .metallic_factor = static_cast<float>(material.pbr_metallic_roughness.metallic_factor),
            .normal_texture_index = Scene::get_texture_index_with_black_fallback(material.normal_texture.index),
            .normal_texture_scale = static_cast<float>(material.normal_texture.scale),
            .occlusion_texture_index = Scene::get_texture_index_with_white_fallback(material.occlusion_texture.index),
            .occlusion_texture_strength = static_cast<float>(material.occlusion_texture.strength),
            .emissive_texture_index = Scene::get_texture_index_with_black_fallback(material.emissive_texture.index),
            .emissive_texture_factor =
                glm::vec3(material.emissive_factor[0], material.emissive_factor[1], material.emissive_factor[2]),
        };

        scene.materials.emplace_back(material_data);
    }
}
auto GltfLoader::load_images(SceneData& scene, const tg3_model& model) -> void {
    for (size_t i = 0; i < model.images_count; i++) {
        const tg3_image image = model.images[i];
        std::string_view mime_type = tg3_str_to_string_view(image.mime_type);

        Image image_data;

        if (image.uri.data == nullptr && image.buffer_view != -1) {
            // the texture is stored inside the file itself
            const tg3_buffer_view& buffer_view = model.buffer_views[image.buffer_view];
            const tg3_buffer& buffer = model.buffers[buffer_view.buffer];

            std::span<const uint8_t> image_bytes =
                std::span<const uint8_t>(buffer.data.data + buffer_view.byte_offset, buffer_view.byte_length);

            image_data = load_image_with_mime_type_from_bytes(image_bytes, mime_type);
        } else if (image.uri.data == nullptr && image.buffer_view != -1) {
            // the texture is an external file
            std::string_view image_uri = tg3_str_to_string_view(image.uri);
            std::vector<uint8_t> bytes = FileUtils::read_file(std::string(image_uri));

            image_data = load_image_with_mime_type_from_bytes(bytes, mime_type);
        } else {
            throw std::runtime_error("failed to load texture");
        }

        auto image_meta = ImageData {
            .width = image_data.width,
            .height = image_data.height,
            .format = image_data.format,
            .data_span = IndexSpan {
                .index = static_cast<uint32_t>(scene.image_data.size()),
                .length = static_cast<uint32_t>(image_data.data.size()),
            },
        };

        scene.image_data.append_range(image_data.data);
        scene.images.emplace_back(image_meta);
    }
}

auto GltfLoader::load_textures(SceneData& scene, const tg3_model& model) -> void {
    for (size_t i = 0; i < model.textures_count; i++) {
        const tg3_texture& texture = model.textures[i];

        auto texture_data = Texture {
            .image_index = static_cast<uint32_t>(texture.source),
            .sampler_index = static_cast<uint32_t>(texture.sampler),
        };

        scene.textures.emplace_back(texture_data);
    }
}

auto GltfLoader::weave_vertex_data_from_accessors(
    const AccessorView<glm::vec3> positions,
    const AccessorView<glm::vec2> texture_coords,
    const AccessorView<glm::vec3> normals,
    const tg3_model& model
) -> std::vector<Vertex> {
    auto vertices = std::vector<Vertex>();
    vertices.reserve(positions.length);

    for (size_t i = 0; i < positions.length; i++) {
        const glm::vec3* position = positions.get(i);
        const glm::vec2* texture_coord = texture_coords.get(i);
        const glm::vec3* normal = normals.get(i);

        auto vertex = Vertex {
            .position = *position,
            .texture_coord = *texture_coord,
            .normal = *normal,
            .tangent = glm::vec3(0),
        };

        vertices.emplace_back(vertex);
    }

    return vertices;
}

auto GltfLoader::get_material_index_with_default_fallback(SceneData& data, int32_t index) -> uint32_t {
    return 0;  // TODO!
}
auto GltfLoader::tg3_str_to_string_view(const tg3_str& str) -> std::string_view {
    return std::string_view(str.data, str.len);
}
auto GltfLoader::load_image_with_mime_type_from_bytes(std::span<const uint8_t> bytes, std::string_view mime_type)
    -> Image {
    Image image;

    if (mime_type == "image/ktx2") {
        image = KtxImage::from_bytes(bytes);
    } else if (mime_type == "image/png" || mime_type == "image/jpeg") {
        image = StbImage::from_bytes(bytes);
    }

    if (image.data.size() == 0 || (image.width == 0 && image.height == 0)) {
        throw std::runtime_error(std::format("Error when loading an image"));
    }

    return image;
}
