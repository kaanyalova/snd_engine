// static float2 positions[3] = float2[](float2(0.0, -0.5), float2(0.5, 0.5), float2(-0.5, 0.5));

// static float3 colors[3] =
//     float3[](float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0));

struct VertexInput {
    float2 position;
    float3 color;
    float2 texture_coord;
}

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

ConstantBuffer<UniformBuffer> uniform_buffer;

[shader("vertex")]
VertexOutput vertex_main(VertexInput input) {
    VertexOutput output;

    float4x4 mvp = mul(uniform_buffer.projection, mul(uniform_buffer.view, uniform_buffer.model));

    output.position = mul(mvp, float4(input.position, 0.0, 1.0));
    output.color = input.color;
    output.texture_coord = input.texture_coord;

    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragment_main(VertexOutput vertex_output) : SV_Target {
    return texture.Sample(vertex_output.texture_coord);
}
