#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../src/engine/cameras/camera.h"

#include <SDL3/SDL.h>

class CameraTest : public ::testing::Test {
   protected:
	void SetUp() override {
		Camera cam;
		cam.name = "test_cam";
		cam.transform.position = glm::vec3(0.0f);
		cam.transform.rotation = glm::quat(1, 0, 0, 0);
		cam.move_speed = 1.0f;
		cam.look_sensitivity = 1.0f;

		manager.add_camera(cam);
		manager.set_active_camera(&manager.get_camera("test_cam")[0]);
	}

	CameraManager manager;
};

TEST_F(
	CameraTest,
	RetrieveCamera
) {
	Camera* cam = manager.get_camera("test_cam");
	ASSERT_NE(cam, nullptr) << "Expected camera to not be null";
	EXPECT_EQ(cam->name, "test_cam") << "Expected camera to have name";
}

TEST_F(
	CameraTest,
	ReturnsNullForMissingCamera
) {
	Camera* cam = manager.get_camera("null_cam");
	ASSERT_EQ(cam, nullptr) << "Expected camera to be null";
}

TEST_F(
	CameraTest,
	GetActiveCamera
) {
	Camera* active = manager.get_active_camera();
	ASSERT_NE(active, nullptr) << "Expected camera to not be null";
	EXPECT_EQ(active->name, "test_cam") << "Expected camera to have name";
}

TEST_F(
	CameraTest,
	UpdatePositionForward
) {
	Camera* cam = manager.get_active_camera();
	glm::vec3 start_pos = cam->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_W] = true;

	manager.update_camera_position(1.0f, keys);

	EXPECT_LT(cam->transform.position.z, start_pos.z) << "Expected camera "
														 "position to move "
														 "backward";
}

TEST_F(
	CameraTest,
	UpdatePositionBackward
) {
	Camera* cam = manager.get_active_camera();
	glm::vec3 start_pos = cam->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_S] = true;

	manager.update_camera_position(1.0f, keys);

	EXPECT_GT(cam->transform.position.z, start_pos.z) << "Expected camera "
														 "position to move "
														 "forward";
}

TEST_F(
	CameraTest,
	UpdatePositionRight
) {
	Camera* cam = manager.get_active_camera();
	glm::vec3 start_pos = cam->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_D] = true;

	manager.update_camera_position(1.0f, keys);

	EXPECT_GT(cam->transform.position.x, start_pos.x) << "Expected camera "
														 "position to move "
														 "right";
}

TEST_F(
	CameraTest,
	UpdatePositionLeft
) {
	Camera* cam = manager.get_active_camera();
	glm::vec3 start_pos = cam->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_A] = true;

	manager.update_camera_position(1.0f, keys);

	EXPECT_LT(cam->transform.position.x, start_pos.x) << "Expected camera "
														 "position to move "
														 "left";
}

TEST_F(
	CameraTest,
	UpdateLookChangesRotation
) {
	Camera* cam = manager.get_active_camera();
	glm::quat before = cam->transform.rotation;

	MouseInput mouse{10.0f, -5.0f};

	manager.update_camera_look(&mouse, cam);
	glm::quat after = cam->transform.rotation;

	// The rotation should have changed
	EXPECT_FALSE(glm::all(glm::equal(before, after))) << "Expected camera "
														 "rotation to change";
}