#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#include <unordered_map>
#include <glm.hpp>
#include <gtc/quaternion.hpp>

#include "entity.h"
#include "inputs.h"

struct Lens {
	float fov;
	float aspect;
	float near_clip;
	float far_clip;
};

struct Camera {
	std::string name;
	Transform transform;
	Lens lens;
	float move_speed;
	float look_sensitivity;
};


class CameraManager {
public:
	void add_camera(const Camera& camera);
	Camera* get_camera(const std::string& name);

	void set_active_camera(const Camera *camera);
	Camera* get_active_camera();

	void update_camera_position(float delta_time, const bool* keys);
	void update_camera_look(const MouseInput *mouse_input, Camera *camera);
private:
	std::unordered_map<std::string, Camera> cameras;
	const Camera* active_camera = nullptr;
};


#endif	// CAMERA_H
