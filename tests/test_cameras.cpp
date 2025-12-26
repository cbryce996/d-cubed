#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../src/engine/cameras/camera.h"

#include <SDL3/SDL.h>

// Simple helper to compare vectors with tolerance
bool approx_equal(const glm::vec3& a, const glm::vec3& b, float epsilon = 1e-5f) {
	return glm::all(glm::lessThan(glm::abs(a - b), glm::vec3(epsilon)));
}

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
		manager.set_active_camera(&manager.get_camera("test_cam")[0]); // pointer
	}

	CameraManager manager;
};

TEST_F(CameraTest, AddAndRetrieveCamera) {
	Camera* cam = manager.get_camera("test_cam");
	ASSERT_NE(cam, nullptr);
	EXPECT_EQ(cam->name, "test_cam");
}

TEST_F(CameraTest, ActiveCameraSetAndGet) {
	Camera* active = manager.get_active_camera();
	ASSERT_NE(active, nullptr);
	EXPECT_EQ(active->name, "test_cam");
}

TEST_F(CameraTest, UpdatePositionForward) {
	Camera* cam = manager.get_active_camera();
	bool keys[SDL_SCANCODE_COUNT + 1] = {false}; // SDL3
	keys[SDL_SCANCODE_W] = true;

	manager.update_camera_position(1.0f, keys);

	EXPECT_FALSE(approx_equal(cam->transform.position, glm::vec3(0.0f,0.0f,0.0f)));
}

TEST_F(CameraTest, UpdatePositionRight) {
	Camera* cam = manager.get_active_camera();
	bool keys[SDL_SCANCODE_COUNT + 1] = {false}; // SDL3
	keys[SDL_SCANCODE_D] = true;

	manager.update_camera_position(1.0f, keys);

	EXPECT_GT(cam->transform.position.x, 0.0f);
}

TEST_F(CameraTest, UpdateLookChangesRotation) {
	Camera* cam = manager.get_active_camera();
	MouseInput mouse{10.0f, -5.0f};

	glm::quat before = cam->transform.rotation;
	manager.update_camera_look(&mouse, cam);

	EXPECT_FALSE(glm::all(glm::equal(before, cam->transform.rotation)));
}
