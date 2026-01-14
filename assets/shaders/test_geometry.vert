#version 450

layout(binding = 0) uniform ViewUniform {
    mat4 view_projection;
} view_u;

layout(location = 0) in vec4 inMeshPos;
layout(location = 1) in vec4 inMeshNormal;
layout(location = 2) in vec4 inMeshColor;
layout(location = 3) in vec4 inMeshPad;

layout(location = 4) in vec4 inInstPos;
layout(location = 5) in vec4 inInstRot;
layout(location = 6) in vec4 inInstScale;
layout(location = 7) in vec4 inInstPad;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;

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

    float a = angle;
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

vec3 transformMesh(
    vec3 position,
    vec3 scale,
    mat3 rotation
) {
    return rotation * (position * scale);
}

void main() {
    vec3 instancePos = inInstPos.xyz;
    vec3 scale       = inInstScale.xyz;
    vec3 axis        = inInstRot.xyz;
    float angle      = inInstPos.w;

    mat3 rotation = quaternionRotation(axis, angle);

    vec3 localPos = inMeshPos.xyz * scale;
    vec3 worldPos = instancePos + rotation * localPos;

    vec3 worldNormal = normalize(rotation * inMeshNormal.xyz);

    outPosition = vec4(worldPos, 1.0);
    outNormal   = vec4(worldNormal, 0.0);
    outColor    = vec4(inMeshColor.rgb, 1.0);

    gl_Position = view_u.view_projection * vec4(worldPos, 1.0);
}
