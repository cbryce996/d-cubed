#ifndef UTILS_H
#define UTILS_H

#define GLM_ENABLE_EXPERIMENTAL

#include "entity/entity.h"
#include "render/memory.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

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

inline uint64_t generate_uuid64 () {
	static std::mt19937_64 rng{std::random_device{}()};
	static std::uniform_int_distribution<uint64_t> dist;
	return dist (rng);
}

inline void hash_combine (size_t& seed, const size_t value) {
	seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

inline void write_vec4 (Block& block, size_t slot, const glm::vec4& v) {
	assert (slot < 4);
	std::memcpy (&block.data[slot * 4], glm::value_ptr (v), sizeof (glm::vec4));
}

inline void write_mat4 (Block& block, const glm::mat4& m) {
	write_vec4 (block, 0, m[0]);
	write_vec4 (block, 1, m[1]);
	write_vec4 (block, 2, m[2]);
	write_vec4 (block, 3, m[3]);
}

inline IEntity* root_of (IEntity* e) {
	while (e && e->parent)
		e = e->parent;
	return e;
}

static glm::mat3 camera_rotation_mat3 (const Transform& t) {
	const glm::vec3 r = glm::radians (t.rotation);

	glm::mat4 R (1.0f);
	R = R * glm::rotate (glm::mat4 (1.0f), r.x, glm::vec3 (1, 0, 0));
	R = R * glm::rotate (glm::mat4 (1.0f), r.y, glm::vec3 (0, 1, 0));
	R = R * glm::rotate (glm::mat4 (1.0f), r.z, glm::vec3 (0, 0, 1));

	return glm::mat3 (R);
}

#endif // UTILS_H
