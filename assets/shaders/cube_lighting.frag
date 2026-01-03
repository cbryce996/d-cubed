#version 450

layout(set = 0, binding = 0) uniform sampler2D gPositionTex;
layout(set = 0, binding = 1) uniform sampler2D gNormalTex;
layout(set = 0, binding = 2) uniform sampler2D gAlbedoTex;

layout(set = 0, binding = 3) uniform GlobalUniform {
    vec4 light_pos;
    vec4 time;
    vec4 camera_pos;
    vec4 pad2;
} global_u;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

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
    vec3 fragPos = texture(gPositionTex, fragUV).xyz;
    vec3 normal  = texture(gNormalTex, fragUV).xyz;
    vec3 albedo  = texture(gAlbedoTex, fragUV).rgb;

    vec3 lit = applyCameraLight(
    albedo,
    normalize(normal),
    fragPos,
    global_u.camera_pos.xyz
    );

    outColor = vec4(lit, 1.0);
}
