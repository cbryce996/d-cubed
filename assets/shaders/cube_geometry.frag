#version 450

layout(location = 0) in vec4 inFragPos;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec4 inColor;
layout(location = 3) in float inRadius;
layout(location = 4) in float cubeSeed;

/* G-buffer outputs */
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;

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

void main() {
    gPosition = vec4(inFragPos.xyz, 1.0);
    gNormal   = vec4(normalize(inNormal.xyz), 0.0);
    gAlbedo   = vec4(cosmicBlue(cubeSeed), 1.0);
}
