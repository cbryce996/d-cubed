#version 450

// Slot 0: Per-Vertex (The Mesh)
layout(location = 0) in vec3 a_position;

// Slot 1: Per-Instance (The 1,000,000 Cubes)
// We use 4 locations to represent a mat4 (4x vec4)
layout(location = 1) in vec4 i_col0;
layout(location = 2) in vec4 i_col1;
layout(location = 3) in vec4 i_col2;
layout(location = 4) in vec4 i_col3;

layout(set = 0, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
} u_matrices;

void main() {
    // Reconstruct the model matrix from instance attributes
    mat4 instance_model = mat4(i_col0, i_col1, i_col2, i_col3);

    gl_Position = u_matrices.projection * u_matrices.view * instance_model * vec4(a_position, 1.0);
}