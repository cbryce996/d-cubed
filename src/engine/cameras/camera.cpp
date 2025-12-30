#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SDL3/SDL_scancode.h"
#include "utils.h"

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

	const glm::vec3 forward = camera->transform.rotation
							  * glm::vec3 (0.0f, 0.0f, -1.0f);
	const glm::vec3 right = camera->transform.rotation
							* glm::vec3 (1.0f, 0.0f, 0.0f);

	constexpr float max_speed = 0.5f;
	constexpr float min_speed = 0.0f;

	const float unclamped_speed = camera->move_speed * delta_time;
	const float speed = glm::clamp (unclamped_speed, min_speed, max_speed);

	if (keys[SDL_SCANCODE_W])
		camera->transform.position += forward * speed;
	if (keys[SDL_SCANCODE_S])
		camera->transform.position -= forward * speed;
	if (keys[SDL_SCANCODE_A])
		camera->transform.position -= right * speed;
	if (keys[SDL_SCANCODE_D])
		camera->transform.position += right * speed;
}

void CameraManager::update_camera_look (const MouseInput* mouse_input) {
	Camera* camera = get_active_camera ();

	if (!camera)
		return;

	const float yaw_delta = glm::radians (
		-mouse_input->dx * camera->look_sensitivity
	);
	const float pitch_delta = glm::radians (
		-mouse_input->dy * camera->look_sensitivity
	);

	const glm::quat yaw_rotation = glm::angleAxis (
		yaw_delta, glm::vec3 (0, 1, 0)
	);
	const glm::quat pitch_rotation = glm::angleAxis (
		pitch_delta, glm::vec3 (1, 0, 0)
	);

	camera->transform.rotation = yaw_rotation * camera->transform.rotation;
	camera->transform.rotation = camera->transform.rotation * pitch_rotation;
	camera->transform.rotation = glm::normalize (camera->transform.rotation);
}

glm::mat4 CameraManager::compute_view_projection (
	const Camera& camera, float aspect_ratio
) {
	glm::mat4 view = glm::lookAt (
		camera.transform.position,
		camera.transform.position
			+ camera.transform.rotation * glm::vec3 (0.0f, 0.0f, -1.0f),
		camera.transform.rotation * glm::vec3 (0.0f, 1.0f, 0.0f)
	);

	glm::mat4 projection = glm::perspective (
		glm::radians (camera.lens.fov), aspect_ratio, camera.lens.near_clip,
		camera.lens.far_clip
	);
	glm::mat4 view_projection = projection * view;
	return view_projection;
}
