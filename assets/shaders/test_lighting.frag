#version 450

layout(binding = 1) uniform GlobalUniform {
    vec4 light_pos;
    vec4 time;
    vec4 camera_pos;
    vec4 pad2;
} global_u;

layout(binding = 0) uniform sampler2D inPosition;
layout(binding = 3) uniform sampler2D inNormal;
layout(binding = 2) uniform sampler2D inColor;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

// --- Hash functions ---
float hash11(float p) {
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

vec3 cosmicBlue(float seed) {
    float variation = hash11(seed) * 0.5;
    vec3 base = vec3(0.1, 0.2, 0.7);
    return base + variation;
}

// --- Camera lighting ---
vec3 applyCameraLight(vec3 color, vec3 normal, vec3 fragPos, vec3 lightPos) {
    vec3 L = lightPos - fragPos;
    float distance = length(L);
    vec3 lightDir = normalize(L);

    float diff = max(dot(normal, lightDir), 0.0) * 10.0;
    float atten = 1.0 / (distance * 0.8 + 10.0);

    float ambient = 0.3;
    return color * (ambient + diff * atten);
}

// --- Screen-space dithering / aliasing ---
float screenDither(vec2 uv, float seed) {
    float dither = fract(sin(dot(uv * 1234.56, vec2(12.9898, 78.233))) * 43758.5453);
    return mix(0.95, 1.05, dither * hash11(seed)); // subtle brightness variation
}

void main() {
    vec4 positionData = texture(inPosition, inUV);
    vec4 normalData   = texture(inNormal, inUV);
    vec4 colorData    = texture(inColor, inUV);

    vec3 position = positionData.xyz;
    vec3 normal   = normalData.xyz;
    float seed    = colorData.a;

    vec3 baseColor = cosmicBlue(seed);

    // --- Lighting ---
    vec3 litColor = applyCameraLight(baseColor, normal, position, global_u.camera_pos.xyz);

    // Combine everything
    outColor = vec4(litColor, 1.0);
}
