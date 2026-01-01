#include <metal_stdlib>
using namespace metal;

struct ViewUniform {
    float4x4 view_projection;
};

struct GlobalUniform {
    float4 light_pos;
    float4 time;
    float4 pad1;
    float4 pad2;
};

struct VertexIn {
    /* --- Slot 0: Mesh Data (Locations 0-3) --- */
    float4 meshPos    [[attribute(0)]];
    float4 meshNormal [[attribute(1)]];
    float4 meshColor  [[attribute(2)]];
    float4 meshPad    [[attribute(3)]];

    /* --- Slot 1: Instance Data (Locations 4-7) --- */
    float4 instPos    [[attribute(4)]]; // xyz = pos, w = phase
    float4 instRot    [[attribute(5)]]; // xyz = axis, w = unused
    float4 instScale  [[attribute(6)]]; // xyz = scale, w = unused
    float4 instPad    [[attribute(7)]];
};

struct VertexOut {
    float4 position [[position]];
    float3 frag_pos;
    float3 normal;
    float3 color;
};

vertex VertexOut vert_main(
    VertexIn in [[stage_in]],
    constant ViewUniform& view_uniform [[buffer(0)]],
    constant GlobalUniform& global_uniform [[buffer(1)]]
) {
    // --- VERTEX ADJUST PARAMETERS ---
    float noiseScale      = 0.2;   // Frequency of the density noise
    float densityCutoff   = 0.6;   // Higher = thinner nebula/fewer visible cubes
    float densityPower    = 1.5;   // Contrast of the density scaling
    float maxInstanceSize = 0.8;   // Multiplier for individual cube scale
    float orbitIntensity  = 1.5;   // How far cubes stray from their base pos
    float sphereLimit     = 30.0;  // Radius of the total nebula cloud
    // --------------------------------

    float3 basePos = in.instPos.xyz;
    float phase    = in.instPos.w;
    float3 axis    = normalize(in.instRot.xyz);
    float3 scale   = in.instScale.xyz;

    // 1. Density Logic
    float3 p = basePos * noiseScale;
    float n1 = sin(p.x + global_uniform.time.y * 0.0005)
             * cos(p.y + global_uniform.time.y * 0.0007)
             * sin(p.z);
    float rawDensity = saturate(n1 * 1.5 + 0.3);

    // 2. Constraints (Spherical Mask)
    float distFromCenter = length(basePos);
    float sphericalMask = 1.0 - smoothstep(sphereLimit * 0.6, sphereLimit, distFromCenter);

    // 3. Clipping/Scaling
    float clipped = saturate(rawDensity - densityCutoff) / (1.0 - densityCutoff);
    float sizeMod = mix(0.0, maxInstanceSize, pow(clipped, densityPower)) * sphericalMask;

    // 4. Animation
    float t = global_uniform.time.x * 0.0005 + phase;
    float3 offset = float3(sin(t), cos(t), sin(t)) * orbitIntensity;

    // 5. Rotation (Rodrigues)
    float angle = t * 0.5;
    float c = cos(angle), s = sin(angle), mc = 1.0 - c;
    float3x3 rot = float3x3(
        float3(c + axis.x*axis.x*mc, axis.x*axis.y*mc - axis.z*s, axis.x*axis.z*mc + axis.y*s),
        float3(axis.y*axis.x*mc + axis.z*s, c + axis.y*axis.y*mc, axis.y*axis.z*mc - axis.x*s),
        float3(axis.z*axis.x*mc - axis.y*s, axis.z*axis.y*mc + axis.x*s, c + axis.z*axis.z*mc)
    );

    VertexOut out;
    float3 worldPos = rot * (in.meshPos.xyz * scale * sizeMod) + basePos + offset;

    out.position = view_uniform.view_projection * float4(worldPos, 1.0);
    out.frag_pos = worldPos;
    out.color = in.meshColor.xyz;

    // Normal calculation: Blend mesh normal with "clump center" normal for softer look
    float3 cubeNormal = rot * in.meshNormal.xyz;
    float3 clumpNormal = normalize(worldPos - basePos);
    out.normal = normalize(mix(cubeNormal, clumpNormal, 0.7));

    return out;
}

fragment float4 frag_main(
    VertexOut in [[stage_in]],
    constant GlobalUniform& global_uniform [[buffer(1)]]
) {
    // --- FRAGMENT ADJUST PARAMETERS ---
    float densitySharpness = 0.6;  // < 1.0 "bloats" colors, > 1.0 thins them
    float colorSpread      = 0.8;  // How far the mid-color stretches
    float globalGlow       = 1.0;  // Overall brightness
    float shadowLift       = 0.5;  // Ambient light level (0.0 to 1.0)
    float fresnelPower     = 2.0;  // Edge glow tightness
    float fresnelIntensity = 0.5;  // Edge glow brightness
    // ----------------------------------

    // 1. Fragment-level Noise (Micro-detail)
    float3 p = in.frag_pos * 0.2;
    float3 p2 = in.frag_pos * 2.0;
    float noise1 = sin(p.x + global_uniform.time.y * 0.0005) * cos(p.y + global_uniform.time.y * 0.0007) * sin(p.z);
    float noise2 = sin(p2.x - global_uniform.time.y * 0.001) * cos(p2.y) * sin(p2.z + global_uniform.time.y * 0.001);

    float rawDensity = saturate((noise1 * 0.7 + noise2 * 0.3) * 1.2 + 0.3);
    float density = pow(rawDensity, densitySharpness);

    // 2. Color Ramp
    float3 minColor = float3(0.05, 0.02, 0.1); // Deep Shadows
    float3 midColor = float3(0.2, 0.5, 0.8);   // Primary Nebula Blue
    float3 maxColor = float3(0.7, 1.0, 1.0);   // Core Highlights

    float3 baseColor = mix(minColor, midColor, saturate(density * colorSpread));
    baseColor = mix(baseColor, maxColor, saturate((density - 0.6) * 2.0));

    // 3. Lighting
    float3 N = normalize(in.normal);
    float3 L = normalize(global_uniform.light_pos.xyz - in.frag_pos);
    float3 V = normalize(-in.frag_pos);

    float diff = max(dot(N, L), shadowLift);
    float fresnel = pow(1.0 - max(dot(N, V), 0.0), fresnelPower);

    // 4. Final Color
    float3 finalColor = baseColor * (diff + fresnel * fresnelIntensity) * globalGlow;

    return float4(saturate(finalColor), 1.0);
}