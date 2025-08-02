#include <metal_stdlib>
using namespace metal;

struct Uniform {
    float4x4 model;     // NEW: model matrix
    float4x4 mvp;       // model * view * projection
    float3   light_pos; // now in world space
    float    pad;       // for alignment
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float3 color    [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 color;
    float3 normal;   // world-space normal
    float3 frag_pos; // world-space fragment position
};

vertex VertexOut vert_main(VertexIn in [[stage_in]],
                           constant Uniform& ubo [[buffer(0)]]) {
    VertexOut out;

    float3x3 normal_matrix = float3x3(
        ubo.model[0].xyz,
        ubo.model[1].xyz,
        ubo.model[2].xyz
    );

    float4 local_pos = float4(in.position, 1.0);
    float4 world_pos = ubo.model * local_pos;

    out.position = ubo.mvp * local_pos;     // still use mvp for screen-space output
    out.frag_pos = world_pos.xyz;           // world-space position
    out.normal = normalize(normal_matrix * in.normal);
    out.color = in.color;

    return out;
}

fragment float4 frag_main(VertexOut in [[stage_in]],
                          constant Uniform& ubo [[buffer(0)]]) {
    float3 L = normalize(ubo.light_pos - in.frag_pos);
    float3 N = normalize(in.normal);

    float diff = max(dot(N, L), 0.0);

    float strength = 1;
    float distance = length(ubo.light_pos - in.frag_pos);

    float attenuation = 1.0 / (1 + distance * distance); // softer near the light

    float3 lit_color = clamp(in.color * diff * attenuation * strength, 0.0, 0.8);
    return float4(lit_color, 1.0);
}