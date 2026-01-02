#version 450

layout(set = 0, binding = 1) uniform GlobalUniform {
    vec4 light_pos;
    vec4 time;
    vec4 pad1;
    vec4 pad2;
} global_u;

layout(location = 0) in vec4 inFragPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in float inRadius;

layout(location = 0) out vec4 outFragColor;

// --- Generic Math Functions ---

// Smoothstep interpolation (ease in/out)
float smoothInterp(float t) {
    return t * t * (3.0 - 2.0 * t);
}

// Inverse-square attenuation with linear and constant terms
float inverseQuadratic(float distance, float constant, float linear, float quadratic) {
    return 1.0 / (constant + linear * distance + quadratic * distance * distance);
}

// Dot product clamped to [0,1] (projection magnitude)
float projectClamp(vec3 a, vec3 b) {
    return max(dot(a, b), 0.0);
}

// Fresnel-like edge factor (1 - cos angle)^power
float edgeFactor(vec3 normal, vec3 viewDir, float power) {
    return pow(1.0 - max(dot(normal, viewDir), 0.0), power);
}

// Multi-step gradient with smooth interpolation
vec3 multiGradient(float t) {
    float u = smoothInterp(t);

    vec3 c0 = vec3(0.2, 0.0, 0.4);
    vec3 c1 = vec3(0.2, 0.4, 0.8);
    vec3 c2 = vec3(0.6, 0.8, 1.0);

    float blend0 = clamp(1.0 - u * 2.0, 0.0, 1.0);
    float blend2 = clamp(u * 2.0 - 1.0, 0.0, 1.0);
    float blend1 = 1.0 - blend0 - blend2;

    return c0 * blend0 + c1 * blend1 + c2 * blend2;
}

// --- Main Shader ---
void main() {
    vec3 fragPos = inFragPos.xyz;
    vec3 normal = normalize(inNormal.xyz);
    vec3 viewDir = normalize(-fragPos);

    vec3 lightPos = global_u.light_pos.xyz;
    vec3 L = lightPos - fragPos;
    float distance = length(L);
    vec3 lightDir = normalize(L);

    float rawAtten = inverseQuadratic(distance, 1.0, 0.5, 1.0);
    float lightStrength = 100.0f;
    float attenuation = min(rawAtten * lightStrength, 0.6f); // clamp to max 1

    float diffuse = projectClamp(normal, lightDir) * attenuation;
    float maxDiffuse = 0.7f;
    diffuse = min(diffuse, maxDiffuse);

    float t = clamp(diffuse, 0.0, 1.0);
    vec3 baseColor = multiGradient(t);

    float rim = edgeFactor(normal, viewDir, 3.0);
    vec3 finalColor = baseColor + rim * 0.01 * vec3(0.8, 0.6, 1.0);

    outFragColor = vec4(finalColor, 1.0);
}
