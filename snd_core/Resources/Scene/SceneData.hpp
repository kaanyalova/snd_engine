#pragma once

#include <glm/ext/vector_float4.hpp>
#include <glm/glm.hpp>
#include <span>
#include <string>
#include <vector>

#include "snd_core/Image/Image.hpp"

struct Vertex {
    glm::vec3 position;
    glm::vec2 texture_coord;
    glm::vec3 normal;
    glm::vec3 tangent;
};

struct PrimitiveInstance {
    glm::mat4x4 transform;
    uint32_t primitive_index;
};

struct IndexSpan {
    uint32_t index;
    uint32_t length;

    auto end_index() -> uint32_t { return index + length; }
};

struct Primitive {
    uint32_t material_index;
    IndexSpan vertex_span;
};

struct MaterialData {
    uint32_t base_color_texture_index;
    glm::vec4 base_color_texture_factor;
    uint32_t metallic_roughness_texture_index;
    float roughness_factor;
    float metallic_factor;
    uint32_t normal_texture_index;
    float normal_texture_scale;
    uint32_t occlusion_texture_index;
    float occlusion_texture_strength;
    uint32_t emissive_texture_index;
    glm::vec3 emissive_texture_factor;
};

struct Texture {
    uint32_t image_index;
    uint32_t sampler_index;
};

struct SamplerData {
    int32_t min_filter;
    int32_t mag_filter;
    int32_t wrap_s;
    int32_t wrap_t;
};

struct ModelRange {
    IndexSpan primitive_instances;
    IndexSpan primitives;
    IndexSpan materials;
    IndexSpan textures;
    IndexSpan samplers;
    IndexSpan vertices;
    IndexSpan indices;
    IndexSpan image_data;
    IndexSpan images;
};

struct ImageData {
    uint32_t width;
    uint32_t height;
    ImageFormat format;
    IndexSpan data_span;

    auto view_of(std::span<const uint8_t> data) const -> ImageView;
};

struct SceneData {
    std::vector<PrimitiveInstance> primitive_instances = {};
    std::vector<Primitive> primitives = {};
    std::vector<MaterialData> materials = {};
    std::vector<ImageData> images = {};
    std::vector<Texture> textures = {};
    std::vector<SamplerData> samplers = {};
    std::vector<Vertex> vertices = {};
    std::vector<uint32_t> indices = {};
    std::vector<uint8_t> image_data = {};

    auto get_scene_stats() -> std::string;
};