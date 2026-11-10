module DataTypes;

public struct Vertex {
    public float3 position;
    public float2 texture_coord;
    public float3 normal;
    public float3 tangent;
};


public struct IndexSpan {
    public uint index;
    public uint length;
};


// matches BakedPrimitiveInstance
public struct PrimitiveInstance {
    public uint primitive_index;
    public uint node_transform_index;
    public float4x4 transform;
};


// matches MaterialData
public struct Material {
    public uint base_color_texture_index;
    public float4 base_color_texture_factor;
    public uint metallic_roughness_texture_index;
    public float roughness_factor;
    public float metallic_factor;
    public uint normal_texture_index;
    public float normal_texture_scale;
    public uint occlusion_texture_index;
    public float occlusion_texture_strength;
    public uint emissive_texture_index;
    public float3 emissive_texture_factor;
};


// TODO: what the fuck is this? just get rid of it?
public struct Primitive {
    public uint material_index;
    public IndexSpan vertex_span;
    public IndexSpan instances;
};

public struct Camera {

};

public struct SceneData {
    public Vertex* vertices;
    public float4x4* scene_transforms;
    public PrimitiveInstance* primitive_instances;
    public Primitive* primitives;
    public Material* materials;
    public Camera* cameras;
};

public struct PushConstants {
    public SceneData* scene_data;
};


public struct VertexOutput {
    public float3 color;
    public float4 position : SV_Position;
    public float2 texture_coord;
};

public struct UniformBuffer {
    public float4x4 model;
    public float4x4 view;
    public float4x4 projection;
}
