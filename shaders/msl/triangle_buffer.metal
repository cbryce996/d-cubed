#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]];
};

vertex VertexOut vert_main(VertexIn in [[stage_in]]) {
    VertexOut out;
    out.position = float4(in.position, 1.0);
    return out;
}

fragment float4 frag_main(VertexOut in [[stage_in]]) {
    return float4(1.0, 0.8, 0.2, 1.0); // gold-ish
}
