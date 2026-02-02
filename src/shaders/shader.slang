static float2 positions[3] = float2[](float2(0.0, -0.5), float2(0.5, 0.5), float2(-0.5, 0.5));

static float3 colors[3] =
    float3[](float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0));

struct VertexInput {
    float2 position;
    float3 color;
}

struct VertexOutput {
    float3 color;
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOutput vertex_main(VertexInput input) {
    VertexOutput output;

    output.position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragment_main(VertexOutput vertex_output) : SV_Target {
    return float4(vertex_output.color, 1.0);
}

