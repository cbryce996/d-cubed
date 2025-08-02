#include <metal_stdlib>
using namespace metal;

struct Uniform {
    float4x4 mvp;
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float3 color    [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 color;
};

vertex VertexOut vert_main(VertexIn in [[stage_in]],
                           constant Uniform& ubo [[buffer(0)]]) {
    VertexOut out;
    out.position = ubo.mvp * float4(in.position, 1.0);
    out.color = in.color;
    return out;
}

fragment float4 frag_main(VertexOut in [[stage_in]]) {
    return float4(in.color, 1.0);
}
