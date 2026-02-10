#version 450

layout(set=0, binding=0) uniform ViewUniform {
    mat4 view_projection;
} view_u;

layout(location = 0) in vec4 inMeshPos;
layout(location = 1) in vec4 inMeshNormal;
layout(location = 2) in vec4 inMeshColor;
layout(location = 3) in vec4 inMeshPad;

layout(location = 4) in vec4 inInstCol0;
layout(location = 5) in vec4 inInstCol1;
layout(location = 6) in vec4 inInstCol2;
layout(location = 7) in vec4 inInstCol3;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;

void main() {
    mat4 model = mat4(inInstCol0, inInstCol1, inInstCol2, inInstCol3);

    vec4 worldPos = model * vec4(inMeshPos.xyz, 1.0);

    mat3 normalMat = mat3(transpose(inverse(model)));
    vec3 worldNormal = normalize(normalMat * inMeshNormal.xyz);

    outPosition = worldPos;
    outNormal   = vec4(worldNormal, 0.0);
    outColor    = vec4(inMeshColor.rgb, 1.0);

    gl_Position = view_u.view_projection * worldPos;
}
