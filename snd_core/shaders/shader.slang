// static float2 positions[3] = float2[](float2(0.0, -0.5), float2(0.5, 0.5), float2(-0.5, 0.5));

// static float3 colors[3] =
//     float3[](float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0));

module MainShader;
import DataTypes;

[shader("vertex")]
VertexOutput vertex_main(
    uniform PushConstants push_constants,
    uint instance_offset: SV_InstanceID,
    uint instance_start: SV_BaseInstanceID,
    uint vertex_offset: SV_VertexID,
    uint vertex_start: SV_StartVertexLocation
) {
    uint instance_index = instance_start + instance_offset;
    uint vertex_index = vertex_start + vertex_offset;
    SceneData scene_data = push_constants.scene_data;

    PrimitiveInstance instance = scene_data.primitive_instances[instance_index];
    Primitive primitive = scene_data.primitives[instance.primitive_index];

    float4x4 scene_transform = scene_data.scene_transforms[instance.node_transform_index];
    Material material = scene_data.materials[primitive.material_index];
    Vertex vertex = scene_data.vertices[vertex_index];

    VertexOutput output;
    float4x4 mvp = mul(uniform_buffer.projection, mul(uniform_buffer.view, uniform_buffer.model));
    output.position = mul(mvp, float4(input.position, 0.0, 1.0));
    output.color = input.color;
    output.texture_coord = input.texture_coord;

    return output;
}


[shader("fragment")]
float4 fragment_main(VertexOutput vertex_output) : SV_Target {
    return texture.Sample(vertex_output.texture_coord);
}
