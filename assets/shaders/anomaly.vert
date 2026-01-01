#version 450

layout(set = 0, binding = 0) uniform ViewUniform {
    mat4 view_projection;
} view_u;

layout(set = 0, binding = 1) uniform GlobalUniform {
    vec4 light_pos;
    vec4 time;
    vec4 pad1;
    vec4 pad2;
} global_u;

// Force vec4 for everything to match C++ Collection packing
layout(location = 0) in vec4 inMeshPos;
layout(location = 1) in vec4 inMeshNormal;
layout(location = 2) in vec4 inMeshColor;
layout(location = 3) in vec4 inMeshPad;

layout(location = 4) in vec4 inInstPos;
layout(location = 5) in vec4 inInstRot;
layout(location = 6) in vec4 inInstScale;
layout(location = 7) in vec4 inInstPad;

layout(location = 0) out vec4 outFragPos;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;

void main() {
    vec3 basePos = inInstPos.xyz;
    float phase  = inInstPos.w;
    vec3 axis    = normalize(inInstRot.xyz);
    vec3 scale   = inInstScale.xyz;

    // 1. Density Logic (Animation)
    float t = global_u.time.x * 0.001 + phase; // Use time.x
    vec3 p = basePos * 0.2;
    float n1 = sin(p.x + t * 0.5) * cos(p.y + t * 0.7) * sin(p.z);
    float rawDensity = clamp(n1 * 1.5 + 0.3, 0.0, 1.0);

    float sizeMod = mix(0.1, 0.8, pow(rawDensity, 1.5));
    vec3 offset = vec3(sin(t), cos(t), sin(t)) * 1.5;

    // 2. Rodrigues Rotation Matrix (FIXED: Transposed to match Column-Major)
    float angle = t * 0.5;
    float c = cos(angle), s = sin(angle), mc = 1.0 - c;

    // We construct the matrix so it is mathematically correct for GLSL
    mat3 rot = mat3(
    vec3(c + axis.x*axis.x*mc, axis.y*axis.x*mc + axis.z*s, axis.z*axis.x*mc - axis.y*s),
    vec3(axis.x*axis.y*mc - axis.z*s, c + axis.y*axis.y*mc, axis.z*axis.y*mc + axis.x*s),
    vec3(axis.x*axis.z*mc + axis.y*s, axis.y*axis.z*mc - axis.x*s, c + axis.z*axis.z*mc)
    );

    vec3 worldPos = rot * (inMeshPos.xyz * scale * sizeMod) + basePos + offset;

    gl_Position = view_u.view_projection * vec4(worldPos, 1.0);

    // Pass vec4 out to ensure interpolation alignment
    outFragPos = vec4(worldPos, 1.0);
    outColor = vec4(inMeshColor.xyz, 1.0);

    vec3 cubeNormal = rot * inMeshNormal.xyz;
    vec3 clumpNormal = normalize(worldPos - basePos);
    outNormal = vec4(normalize(mix(cubeNormal, clumpNormal, 0.7)), 0.0);
}