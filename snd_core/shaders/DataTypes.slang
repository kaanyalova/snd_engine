implementing DataTypes;

struct Vertex {
    float3 position;
    float2 texture_coord;
    float3 normal;
    float3 tangent;
};


struct Primitive {
    uint vertices_start_index;
    uint vertices_size;
    uint material_index;
};

struct PrimitiveInstance {
    float4x4 transform;
    uint scene_transform_index;
    uint primitive_index;
};

struct Material {
    uint base_color_texture_index;
    float4 base_color_texture_factor;
    uint metallic_roughness_texture_index;
    float roughness_factor;
    float metallic_factor;
    uint normal_texture_index;
    float normal_texture_scale;
    uint occlusion_texture_index;
    float occlusion_texture_strength;
    uint emissive_texture_index;
    float3 emissive_texture_factor;
};



struct SceneData {
    Vertex* vertices;
    float4x4* scene_transforms;
    PrimitiveInstance* primitive_instances;
    Primitive* primitives;
    Material* materials;
}

struct PushConstants {
    SceneData* scene_data;
};


struct VertexOutput {
    float3 color;
    float4 position : SV_Position;
    float2 texture_coord;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
}

