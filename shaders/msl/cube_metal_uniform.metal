#include <metal_stdlib>
using namespace metal;

struct Uniform {
    float4x4 mvp;
};

struct VertexIn {
    float3 position [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]];
};

vertex VertexOut vert_main(VertexIn in [[stage_in]],
                           constant Uniform& ubo [[buffer(0)]]) {
    VertexOut out;
    out.position = ubo.mvp * float4(in.position, 1.0);
    return out;
}

fragment float4 frag_main(VertexOut in [[stage_in]]) {
    return float4(1.0, 0.8, 0.2, 1.0); // gold-ish
}
