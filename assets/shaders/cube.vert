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
layout(location = 3) out float outRadius;

mat3 rodriguesRotation(vec3 axis, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;

    axis = normalize(axis);

    return mat3(
        t*axis.x*axis.x + c,        t*axis.x*axis.y - s*axis.z, t*axis.x*axis.z + s*axis.y,
        t*axis.x*axis.y + s*axis.z, t*axis.y*axis.y + c,        t*axis.y*axis.z - s*axis.x,
        t*axis.x*axis.z - s*axis.y, t*axis.y*axis.z + s*axis.x, t*axis.z*axis.z + c
    );
}

mat3 quaternionRotation(vec3 axis, float angle) {
    axis = normalize(axis);

    float a = angle * 0.5;
    float s = sin(a);

    vec4 q = vec4(axis * s, cos(a));

    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;


    return mat3(
        1.0 - 2.0 * (yy + zz), 2.0 * (xy - wz),       2.0 * (xz + wy),
        2.0 * (xy + wz),       1.0 - 2.0 * (xx + zz), 2.0 * (yz - wx),
        2.0 * (xz - wy),       2.0 * (yz + wx),       1.0 - 2.0 * (xx + yy)
    );
}

float radialFallOff(
    vec3 position,
    vec3 center,
    float radius,
    float power
) {
    float distance = length (position - center);
    float normalized_distance = distance / 15.0f;
    float inverted_distance = clamp(1.0 - normalized_distance, 0.0, 1.0);
    return pow(inverted_distance, power);
}

float sinWave(
    float time,
    float phase,
    float frequency,
    float amplitude
) {
    return sin(phase * frequency + time) * amplitude;
}

vec3 sinWave(
    float time,
    vec3 phase,
    vec3 frequency,
    vec3 amplitude
) {
    return vec3(
        sin(phase.x * frequency.x + time) * amplitude.x,
        cos(phase.y * frequency.y + time) * amplitude.y,
        sin(phase.z * frequency.z + time) * amplitude.z
    );
}

void main() {
    float time = global_u.time.x * 0.001f;
    float phase = inInstRot.w;

    vec3 position = inInstPos.xyz;
    vec3 axis = inInstRot.xyz;

    float frequency = 2.0f * 3.14159f * 0.2f;
    float amplitude = 1.0f;

    mat3 rotationMatrix = quaternionRotation(axis, phase + time);

    float bob = sinWave(time, 0.0f, frequency, amplitude);
    vec3 center = vec3(0.0f) + vec3(0.0f, bob, 0.0f);
    vec3 wobble = sinWave(time + phase, position, vec3(0.5f, 0.7f, 0.9f), vec3(0.4f, 0.6f, 0.2f));

    float sphereBaseRadius = 15.0f;
    float scaleAmount = 5.0f;
    float sphereScale = 1.0f + bob * scaleAmount;

    vec3 sphereCenter = vec3(0.0f, bob, 0.0f);
    float sphereRadius = sphereBaseRadius * sphereScale;

    float instanceScale = inInstScale.x * radialFallOff(position, sphereCenter, sphereRadius, 0.3f);
    vec3 worldPos = position + sphereCenter + wobble + rotationMatrix * (inMeshPos.xyz * instanceScale);
    vec3 worldNormal = normalize(rotationMatrix * inMeshNormal.xyz);

    outFragPos = vec4(worldPos, 1.0f);
    outNormal = vec4(worldNormal, 0.0f);
    outColor = inMeshColor;
    outRadius = sphereRadius;

    gl_Position = view_u.view_projection * vec4(worldPos, 1.0);
}
