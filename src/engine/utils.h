#ifndef UTILS_H
#define UTILS_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "scene/entity.h"

#include <random>

inline glm::vec3
model_to_world (const glm::vec3& local_pos, const glm::mat4& model_matrix) {
	return glm::vec3 (model_matrix * glm::vec4 (local_pos, 1.0f));
}

inline glm::vec3
world_to_view (const glm::vec3& world_pos, const glm::mat4& view_matrix) {
	return glm::vec3 (view_matrix * glm::vec4 (world_pos, 1.0f));
}

inline glm::vec3 model_to_view (
	const glm::vec3& local_pos, const glm::mat4& model, const glm::mat4& view
) {
	return world_to_view (model_to_world (local_pos, model), view);
}

inline glm::vec3
normal_to_world (const glm::vec3& normal, const glm::mat4& model_matrix) {
	glm::mat3 normal_matrix = glm::transpose (
		glm::inverse (glm::mat3 (model_matrix))
	);
	return glm::normalize (normal_matrix * normal);
}

inline glm::vec3 normal_to_view (
	const glm::vec3& normal, const glm::mat4& model_matrix,
	const glm::mat4& view_matrix
) {
	return glm::normalize (
		glm::mat3 (view_matrix) * normal_to_world (normal, model_matrix)
	);
}

inline glm::mat4 compute_model_matrix (const Transform& transform) {
	glm::mat4 translation = glm::translate (
		glm::mat4 (1.0f), transform.position
	);
	glm::mat4 rotation = glm::toMat4 (transform.rotation);
	glm::mat4 scale = glm::scale (glm::mat4 (1.0f), transform.scale);

	return translation * rotation * scale;
}

inline uint64_t generate_uuid64 () {
	static std::mt19937_64 rng{std::random_device{}()};
	static std::uniform_int_distribution<uint64_t> dist;
	return dist (rng);
}

inline void hash_combine (size_t& seed, const size_t value) {
	seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

#endif // UTILS_H
