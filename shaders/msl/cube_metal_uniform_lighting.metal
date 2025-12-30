#include <metal_stdlib>
using namespace metal;

// Uniform buffer (aligned to 16 bytes)
struct ViewUniform {
    float4x4 view_projection;
};

struct GlobalUniform {
    float4 light_pos;
    float4 time;
    float4 pad1;
    float4 pad2;
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
    constant ViewUniform& view_uniform [[buffer(0)]],
    constant GlobalUniform& global_uniform [[buffer(1)]],
    uint instanceID [[instance_id]]
) {
    // 1. Density Logic
    float3 p = in.basePos * 0.2;
    float n1 = sin(p.x + global_uniform.time.y * 0.0005) * cos(p.y + global_uniform.time.y * 0.0007) * sin(p.z);
    float rawDensity = saturate(n1 * 1.5 + 0.3);

    // 2. SPHERICAL CONSTRAINT
    // Forms the overall shape into a large sphere
    float distFromCenter = length(in.basePos);
    float sphereRadius = 40.0;
    float falloff = 15.0;
    float sphericalMask = 1.0 - smoothstep(sphereRadius - falloff, sphereRadius, distFromCenter);

    // 3. CLIPPING (Gaps between clusters)
    // Values below 0.4 density now hit exactly 0 scale
    float threshold = 0.6;
    float clipped = saturate(rawDensity - threshold) / (1.0 - threshold);

    // 4. ANTI-ALIASING SCALE
    float4 clipPos = view_uniform.view_projection * float4(in.basePos, 1.0);
    float farThinning = saturate(1.0 - (clipPos.w * clipPos.w * 0.00002));

    float densityScale = mix(0.0, 3, pow(clipped, 2.5)) * sphericalMask * farThinning;
    float3 finalScale = in.scale * densityScale;

    // 5. INCREASED ORBIT RADIUS`
    // Changed 2.0 to 10.0 for a much wider "swirling" motion
    float t = global_uniform.time.x * 0.0005 + in.phase;
    float orbitRadius = 1.5;
    float3 offset = float3(sin(t) * orbitRadius, cos(t) * orbitRadius, sin(t) * orbitRadius);

    // 6. Transformation
    float angle = t * 0.5;
    float3 axis = normalize(in.rotAxis);
    float c = cos(angle), s = sin(angle), mc = 1.0 - c;
    float3x3 rot = float3x3(
        float3(c + axis.x*axis.x*mc, axis.x*axis.y*mc - axis.z*s, axis.x*axis.z*mc + axis.y*s),
        float3(axis.y*axis.x*mc + axis.z*s, c + axis.y*axis.y*mc, axis.y*axis.z*mc - axis.x*s),
        float3(axis.z*axis.x*mc - axis.y*s, axis.z*axis.y*mc + axis.x*s, c + axis.z*axis.z*mc)
    );

    VertexOut out;
    float3 finalPos = rot * (in.position * finalScale) + in.basePos + offset;
    out.position = view_uniform.view_projection * float4(finalPos, 1.0);
    out.frag_pos = finalPos;
    out.color = in.color;

    // Soft Volumetric Normals
    float3 cubeNormal = rot * in.normal;
    float3 clumpNormal = normalize(finalPos - in.basePos);
    out.normal = normalize(mix(cubeNormal, clumpNormal, 0.7));

    return out;
}

fragment float4 frag_main(
    VertexOut in [[stage_in]],
    constant GlobalUniform& global_uniform [[buffer(1)]]
) {
    // --- TWEAK THESE "KNOBS" ---
    float densitySharpness = 0.6;  // Values < 1.0 "bloat" the noise, filling empty space
    float colorSpread      = 0.8;  // Lower spread creates a more monochromatic, misty feel
    float globalGlow       = 1.0;  // Subtle glow to make the fog feel self-illuminated
    float shadowLift       = 0.6;  // High lift ensures the fog doesn't have harsh black shadows
    float fresnelStrength  = 0.3;
    // ---------------------------

    // 1. Noise logic
    float3 p = in.frag_pos * 0.2;
    float3 p2 = in.frag_pos * 2.0;
    float noise1 = sin(p.x + global_uniform.time.y * 0.0005) * cos(p.y + global_uniform.time.y * 0.0007) * sin(p.z);
    float noise2 = sin(p2.x - global_uniform.time.y * 0.001) * cos(p2.y) * sin(p2.z + global_uniform.time.y * 0.001);

    float rawDensity = saturate((noise1 * 0.7 + noise2 * 0.3) * 1.2 + 0.3);

    // TWEAK: Apply Sharpness
    float density = pow(rawDensity, densitySharpness);

    // 2. Color Setup
    float3 minColor = float3(0.05, 0.02, 0.1); // Deep Space
    float3 midColor = float3(0.2, 0.5, 0.8);   // Electric Blue
    float3 maxColor = float3(0.7, 1.0, 1.0);   // Hot Cyan

    // TWEAK: Apply Spread
    float3 baseColor = mix(minColor, midColor, saturate(density * colorSpread));
    baseColor = mix(baseColor, maxColor, saturate((density - 0.6) * 2.0));

    // 3. Lighting
    float3 N = normalize(in.normal);
    float3 L = normalize(float3(global_uniform.light_pos) - in.frag_pos);
    float3 V = normalize(-in.frag_pos);

    float diff = max(dot(N, L), shadowLift);
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);

    // 4. Final Composition
    // TWEAK: Apply Global Glow
    float3 finalColor = baseColor * (diff + fresnel * fresnelStrength) * globalGlow;

    return float4(saturate(finalColor), 1.0);
}
