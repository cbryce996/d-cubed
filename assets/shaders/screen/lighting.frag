#version 450

layout(set=0, binding=0) uniform GlobalUniform {
    vec4 light_pos;
    vec4 time_params;
    vec4 camera_pos;
} g_u;


layout(set=1, binding = 0) uniform sampler2D inPosition;
layout(set=3, binding = 1) uniform sampler2D inNormal;
layout(set=2, binding = 2) uniform sampler2D inColor;


layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

float hash11(float p) {
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float softMax(float x, float maxVal) {
    return (x * maxVal) / (x + maxVal);
}

vec3 applyCameraLight(vec3 color, vec3 normal, vec3 fragPosition, vec3 lightPosition) {
    vec3 L = lightPosition - fragPosition;
    float distance = length(L);
    vec3 lightDir = L / max(distance, 1e-6);

    float diff = max(dot(normal, lightDir), 0.0) * 30.0;
    float atten = 1.0 / (distance * 1.0 + 10.0);

    float range    = 100.0;
    float maxLight = 0.9;
    float ambient  = 0.5;

    float rangeAtten = 1.0 - smoothstep(0.0, range, distance);

    float lighting = ambient + diff * atten * rangeAtten;

    lighting = softMax(lighting, maxLight);

    return color * lighting;
}

float screenDither(vec2 uv, float seed) {
    float dither = fract(sin(dot(uv * 1234.56, vec2(12.9898, 78.233))) * 43758.5453);
    return mix(0.95, 1.05, dither * hash11(seed));
}

void main() {
    vec4 positionData = texture(inPosition, inUV);
    vec4 normalData   = texture(inNormal, inUV);
    vec4 colorData    = texture(inColor, inUV);

    if (length(normalData.xyz) == 0.0) {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 position = positionData.xyz;
    vec3 normal   = normalize(normalData.xyz);

    vec3 baseColor = vec3(1.0);

    vec3 litColor = applyCameraLight(
        baseColor,
        normal,
        position,
        g_u.camera_pos.xyz
    );

    outColor = vec4(litColor, 1.0);
}