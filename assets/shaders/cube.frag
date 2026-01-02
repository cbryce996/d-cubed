#version 450

layout(set = 0, binding = 1) uniform GlobalUniform {
    vec4 light_pos;
    vec4 time;
    vec4 camera_pos;
    vec4 pad2;
} global_u;

layout(location = 0) in vec4 inFragPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in float inRadius;
layout(location = 4) in float cubeSeed;

layout(location = 0) out vec4 outFragColor;

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

vec3 applyCameraLight(vec3 color, vec3 normal, vec3 fragPos, vec3 lightPos) {
    vec3 L = lightPos - fragPos;
    float distance = length(L);
    vec3 lightDir = normalize(L);

    float diff = max(dot(normal, lightDir), 0.0) * 10.0;
    float atten = 1.0 / (distance * 0.8 + 10.0);

    float ambient = 0.3;
    return color * (ambient + diff * atten);
}

void main() {
    vec3 fragPos = inFragPos.xyz;
    vec3 normal = normalize(inNormal.xyz);
    vec3 baseColor = cosmicBlue(cubeSeed);

    vec3 litColor = applyCameraLight(baseColor, normal, fragPos, global_u.camera_pos.xyz);

    // ---------------------
    // simple edge anti-aliasing (screen-space)
    vec3 dFdxNormal = dFdx(normal);
    vec3 dFdyNormal = dFdy(normal);
    float edge = length(dFdxNormal) + length(dFdyNormal);
    edge = clamp(edge * 10.0, 0.0, 1.0); // tweak multiplier for effect

    litColor = mix(litColor, vec3(0.0), edge * 0.5); // darken edges slightly to smooth

    outFragColor = vec4(litColor, 1.0);
}
