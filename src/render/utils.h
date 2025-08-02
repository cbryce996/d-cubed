#ifndef UTILS_H
#define UTILS_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>

#include "camera.h"

struct ModelViewProjection {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 mvp;
};

inline ModelViewProjection compute_model_view_projection(const Camera& camera, float aspect_ratio, const Entity& entity) {
    ModelViewProjection data{};

    // Model matrix from entity's transform
    data.model = glm::translate(glm::mat4(1.0f), entity.transform.position) *
                 glm::toMat4(entity.transform.rotation); // Scale omitted unless needed

    // View matrix from camera (same logic, simplified)
    data.view = glm::lookAt(
        camera.transform.position,
        camera.transform.position + camera.transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f),
        camera.transform.rotation * glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Projection matrix
    data.proj = glm::perspective(
        glm::radians(camera.lens.fov),
        aspect_ratio,
        camera.lens.near_clip,
        camera.lens.far_clip
    );

    // Compose final MVP
    data.mvp = data.proj * data.view * data.model;

    return data;
}

inline glm::vec3 model_to_world(const glm::vec3& local_pos, const glm::mat4& model_matrix) {
    return glm::vec3(model_matrix * glm::vec4(local_pos, 1.0f));
}

inline glm::vec3 world_to_view(const glm::vec3& world_pos, const glm::mat4& view_matrix) {
    return glm::vec3(view_matrix * glm::vec4(world_pos, 1.0f));
}

inline glm::vec3 model_to_view(const glm::vec3& local_pos, const glm::mat4& model, const glm::mat4& view) {
    return world_to_view(model_to_world(local_pos, model), view);
}

inline glm::vec3 normal_to_world(const glm::vec3& normal, const glm::mat4& model_matrix) {
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
    return glm::normalize(normal_matrix * normal);
}

inline glm::vec3 normal_to_view(const glm::vec3& normal, const glm::mat4& model_matrix, const glm::mat4& view_matrix) {
    return glm::normalize(glm::mat3(view_matrix) * normal_to_world(normal, model_matrix));
}


#endif //UTILS_H