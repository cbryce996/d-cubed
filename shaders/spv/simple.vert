#version 450

layout(location = 0) in vec3 a_position;

layout(set = 0, binding = 0) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 projection;
} u_matrices;

void main() {
    gl_Position = u_matrices.projection * u_matrices.view * u_matrices.model * vec4(a_position, 1.0);
}
