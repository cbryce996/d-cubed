#include <metal_stdlib>
using namespace metal;

// Uniform buffer (aligned to 16 bytes)
struct Uniform {
    float4x4 viewProj;
    packed_float3  light_pos;
    float    time;
};

// Per-vertex + per-instance input
struct VertexIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float3 color    [[attribute(2)]];

    // Per-instance data (slot 1 in your vertex descriptor)
    float3 basePos  [[attribute(3)]]; // starting position
    float3 rotAxis  [[attribute(4)]]; // rotation axis
    float  phase    [[attribute(5)]]; // phase offset
    float3 scale    [[attribute(6)]]; // scale
};

// Vertex -> Fragment payload
struct VertexOut {
    float4 position [[position]];
    float3 frag_pos;
    float3 normal;
    float3 color;
};

// Vertex shader (renamed to vert_main)
vertex VertexOut vert_main(
    VertexIn in [[stage_in]],
    constant Uniform& ubo [[buffer(0)]],
    uint instanceID [[instance_id]] // Added instance_id
) {
    // 1. Calculations that are the same for all vertices in this cube
    // These are constant for the instance, but the compiler can optimize
    // internal register usage better when grouped like this.
    float t = ubo.time * 0.001 + in.phase;
    float3 offset = float3(sin(t) * 2.0, cos(t) * 2.0, sin(t) * 2.0);

    float angle = t * 0.5;
    float3 axis = normalize(in.rotAxis);
    float c = cos(angle);
    float s = sin(angle);
    float mc = 1.0 - c;

    // Optimized matrix construction (pre-calculating reused terms)
    float3 row0 = float3(c + axis.x*axis.x*mc,           axis.x*axis.y*mc - axis.z*s,    axis.x*axis.z*mc + axis.y*s);
    float3 row1 = float3(axis.y*axis.x*mc + axis.z*s,    c + axis.y*axis.y*mc,           axis.y*axis.z*mc - axis.x*s);
    float3 row2 = float3(axis.z*axis.x*mc - axis.y*s,    axis.z*axis.y*mc + axis.x*s,    c + axis.z*axis.z*mc);
    float3x3 rot = float3x3(row0, row1, row2);

    // 2. Per-vertex transformations
    VertexOut out;

    // Scale -> Rotate -> Translate
    float3 scaledPos = in.position * in.scale;
    float3 rotatedPos = rot * scaledPos;
    float3 finalPos = rotatedPos + in.basePos + offset;

    float4 world_pos = float4(finalPos, 1.0);

    // 3. Output
    out.position = ubo.viewProj * world_pos;
    out.frag_pos = world_pos.xyz;
    out.color = in.color;

    // Use the same rotation matrix for the normal
    out.normal = rot * in.normal;

    return out;
}

// Fragment shader (renamed to frag_main)
fragment float4 frag_main(
    VertexOut in [[stage_in]],
    constant Uniform& ubo [[buffer(0)]]
) {
    float3 light = float3(ubo.light_pos);
    float3 N = normalize(in.normal);
    float3 L = normalize(light - in.frag_pos);
    float diff = max(dot(N, L), 0.2);

    float distance = length(light - in.frag_pos);
    float d = distance / 5.0;
    float attenuation = 0.8 / (1.0 + d * d);

    float3 ambient = 0.1 * in.color;
    float3 diffuse = in.color * diff * attenuation;

    float3 lit_color = clamp(ambient + diffuse, 0.0, 0.8);
    return float4(lit_color, 1.0);
}
