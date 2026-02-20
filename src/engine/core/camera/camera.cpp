#include "camera.h"
#include "utils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>

#include "core/input/input.h"

CameraManager::CameraManager (const Camera& active_camera) {
	add_camera (active_camera);
	set_active_camera (active_camera.name);
}

void CameraManager::add_camera (const Camera& camera) {
	cameras.emplace (camera.name, camera);
}

Camera* CameraManager::get_camera (const std::string& name) {
	Camera* camera = cameras.contains (name) ? &cameras[name] : nullptr;
	if (!camera) {
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "Camera not found.");
		return nullptr;
	}
	return camera;
}

void CameraManager::set_active_camera (const std::string& name) {
	if (cameras.contains (name)) {
		Camera* camera = get_camera (name);
		active_camera = camera;
	}
}

Camera* CameraManager::get_active_camera () {
	if (!active_camera) {
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "No active camera.");
		return nullptr;
	}
	return get_camera (active_camera->name);
}

void CameraManager::update_camera_position (
	const float delta_time, const bool* keys
) {
	Camera* camera = get_active_camera ();
	if (!camera)
		return;

	constexpr float max_speed = 0.2f;
	const float speed = glm::clamp (
		camera->move_speed * delta_time, 0.0f, max_speed
	);

	const float yaw = glm::radians (camera->transform.rotation.y);
	const float pitch = glm::radians (camera->transform.rotation.x);

	glm::mat4 R (1.0f);
	R = glm::rotate (R, yaw, glm::vec3 (0.0f, 1.0f, 0.0f));
	R = glm::rotate (R, pitch, glm::vec3 (1.0f, 0.0f, 0.0f));

	glm::vec3 forward = glm::normalize (
		glm::vec3 (R * glm::vec4 (0.0f, 0.0f, -1.0f, 0.0f))
	);

	constexpr glm::vec3 worldUp (0.0f, 1.0f, 0.0f);

	glm::vec3 right = glm::cross (forward, worldUp);
	if (glm::length2 (right) < 1e-6f) {
		right = glm::vec3 (1.0f, 0.0f, 0.0f);
	} else {
		right = glm::normalize (right);
	}

	glm::vec3 up = glm::normalize (glm::cross (right, forward));

	glm::vec3 delta (0.0f);

	if (keys[SDL_SCANCODE_W])
		delta += forward;
	if (keys[SDL_SCANCODE_S])
		delta -= forward;
	if (keys[SDL_SCANCODE_D])
		delta += right;
	if (keys[SDL_SCANCODE_A])
		delta -= right;

	if (keys[SDL_SCANCODE_E])
		delta += up;
	if (keys[SDL_SCANCODE_Q])
		delta -= up;

	if (glm::length2 (delta) > 0.0f) {
		delta = glm::normalize (delta);
		camera->transform.position += delta * speed;
	}
}

void CameraManager::update_camera_look (const MouseInput* mouse_input) {
	Camera* camera = get_active_camera ();
	if (!camera)
		return;

	const float yaw_delta = -mouse_input->dx * camera->look_sensitivity;
	const float pitch_delta = mouse_input->dy * camera->look_sensitivity;

	camera->transform.rotation.y += yaw_delta;
	camera->transform.rotation.x += pitch_delta;

	camera->transform.rotation.x = glm::clamp (
		camera->transform.rotation.x, -89.0f, 89.0f
	);

	if (camera->transform.rotation.y > 180.0f)
		camera->transform.rotation.y -= 360.0f;
	if (camera->transform.rotation.y < -180.0f)
		camera->transform.rotation.y += 360.0f;

	camera->transform.rotation.z = 0.0f;
}

glm::mat4 CameraManager::compute_view_projection (
	const Camera& camera, float aspect_ratio
) {
	const glm::vec3 r = glm::radians (camera.transform.rotation);

	glm::mat4 R (1.0f);
	R = glm::rotate (R, r.y, glm::vec3 (0, 1, 0));
	R = glm::rotate (R, r.x, glm::vec3 (1, 0, 0));

	const glm::vec3 forward = glm::normalize (
		glm::vec3 (R * glm::vec4 (0, 0, -1, 0))
	);
	const glm::vec3 up = glm::normalize (
		glm::vec3 (R * glm::vec4 (0, 1, 0, 0))
	);

	const glm::mat4 view = glm::lookAt (
		camera.transform.position, camera.transform.position + forward, up
	);

	const glm::mat4 projection = glm::perspective (
		glm::radians (camera.lens.fov), aspect_ratio, camera.lens.near_clip,
		camera.lens.far_clip
	);

	return projection * view;
}
