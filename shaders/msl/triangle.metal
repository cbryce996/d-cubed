#include <metal_stdlib>
using namespace metal;

vertex float4 vert_main(uint vertexID [[vertex_id]]) {
    float3 pos[3] = {
        float3( 0.0,  0.5, 0.0),
        float3( 0.5, -0.5, 0.0),
        float3(-0.5, -0.5, 0.0)
    };
    return float4(pos[vertexID], 1.0);
}

fragment float4 frag_main() {
    return float4(1.0, 1.0, 0.0, 1.0); // Bright yellow triangle
}
