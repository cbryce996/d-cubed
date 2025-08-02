#ifndef UTILS_H
#define UTILS_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>

#include "camera.h"

struct TransformData {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 mvp;
    glm::mat3 normal_matrix;
};

inline TransformData compute_transform_data(const Camera& camera, float aspect_ratio, const Entity& entity) {
    TransformData data{};

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

inline glm::vec3 compute_light_direction_model_space(const glm::vec3& world_light_dir, const glm::mat4& model_matrix) {
    return glm::normalize(glm::mat3(glm::inverse(model_matrix)) * glm::normalize(world_light_dir));
}



#endif //UTILS_H