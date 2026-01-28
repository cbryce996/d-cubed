#version 450

layout(set=0, binding=0) uniform ViewUniform {
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

mat3 safeQuaternionRotation(vec3 axis, float angle) {
    float len = length(axis);

    if (len < 0.00001) {
        return mat3(1.0);
    }

    axis /= len;

    float s = sin(angle);
    vec4 q = vec4(axis * s, cos(angle));

    float x = q.x, y = q.y, z = q.z, w = q.w;
    float xx = x*x, yy = y*y, zz = z*z;
    float xy = x*y, xz = x*z, yz = y*z;
    float wx = w*x, wy = w*y, wz = w*z;

    return mat3(
    1.0 - 2.0*(yy + zz), 2.0*(xy - wz),       2.0*(xz + wy),
    2.0*(xy + wz),       1.0 - 2.0*(xx + zz), 2.0*(yz - wx),
    2.0*(xz - wy),       2.0*(yz + wx),       1.0 - 2.0*(xx + yy)
    );
}

void main() {
    vec3 instancePos = inInstPos.xyz;
    vec3 scale       = inInstScale.xyz;
    vec3 axis        = inInstRot.xyz;
    float angle      = inInstRot.w;

    mat3 rotation = safeQuaternionRotation(axis, angle);

    vec3 localPos = inMeshPos.xyz * scale;
    vec3 worldPos = instancePos + rotation * localPos;

    vec3 worldNormal = normalize(rotation * inMeshNormal.xyz);

    outPosition = vec4(worldPos, 1.0);
    outNormal   = vec4(worldNormal, 0.0);
    outColor    = vec4(inMeshColor.rgb, 1.0);

    gl_Position = view_u.view_projection * vec4(worldPos, 1.0);
}
