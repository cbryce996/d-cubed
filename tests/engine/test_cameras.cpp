#include "cameras/camera.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "inputs/input.h"

class CameraTest : public ::testing::Test {
  protected:
	void SetUp () override {
		camera->name = "test_cam";
		camera->transform.position = glm::vec3 (0.0f);
		camera->transform.rotation = glm::vec3 (0.0f);
		camera->transform.scale = glm::vec3 (1.0f);

		camera->move_speed = 1.0f;
		camera->look_sensitivity = 1.0f;
	}

	Camera* camera = new Camera ();
};

TEST_F (CameraTest, RetrieveCamera) {
	auto* camera_manager = new CameraManager (*camera);
	camera_manager->add_camera (*camera);

	Camera* retrieved_camera = camera_manager->get_camera ("test_cam");
	ASSERT_NE (retrieved_camera, nullptr);
	EXPECT_EQ (retrieved_camera->name, "test_cam");
}

TEST_F (CameraTest, ReturnsNullForMissingCamera) {
	auto* camera_manager = new CameraManager (*camera);
	Camera* cam = camera_manager->get_camera ("null_cam");
	ASSERT_EQ (cam, nullptr);
}

TEST_F (CameraTest, GetActiveCamera) {
	auto* camera_manager = new CameraManager (*camera);
	camera_manager->add_camera (*camera);
	camera_manager->set_active_camera (camera->name);

	Camera* active_camera = camera_manager->get_active_camera ();
	ASSERT_NE (active_camera, nullptr);
	EXPECT_EQ (active_camera->name, "test_cam");
}

TEST_F (CameraTest, UpdatePositionForward) {
	auto* camera_manager = new CameraManager (*camera);
	camera_manager->add_camera (*camera);
	camera_manager->set_active_camera (camera->name);
	const Camera* active_camera = camera_manager->get_active_camera ();

	const glm::vec3 start_pos = active_camera->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_W] = true;

	camera_manager->update_camera_position (1.0f, keys);

	EXPECT_LT (active_camera->transform.position.z, start_pos.z);
}

TEST_F (CameraTest, UpdatePositionBackward) {
	auto* camera_manager = new CameraManager (*camera);
	camera_manager->add_camera (*camera);
	camera_manager->set_active_camera (camera->name);
	const Camera* active_camera = camera_manager->get_active_camera ();

	const glm::vec3 start_pos = active_camera->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_S] = true;

	camera_manager->update_camera_position (1.0f, keys);

	EXPECT_GT (active_camera->transform.position.z, start_pos.z);
}

TEST_F (CameraTest, UpdatePositionRight) {
	auto* camera_manager = new CameraManager (*camera);
	camera_manager->add_camera (*camera);
	camera_manager->set_active_camera (camera->name);
	const Camera* active_camera = camera_manager->get_active_camera ();

	const glm::vec3 start_pos = active_camera->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_D] = true;

	camera_manager->update_camera_position (1.0f, keys);

	EXPECT_GT (active_camera->transform.position.x, start_pos.x);
}

TEST_F (CameraTest, UpdatePositionLeft) {
	auto* camera_manager = new CameraManager (*camera);
	camera_manager->add_camera (*camera);
	camera_manager->set_active_camera (camera->name);
	const Camera* active_camera = camera_manager->get_active_camera ();

	const glm::vec3 start_pos = active_camera->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_A] = true;

	camera_manager->update_camera_position (1.0f, keys);

	EXPECT_LT (active_camera->transform.position.x, start_pos.x);
}

TEST_F (CameraTest, UpdateLookChangesRotation) {
	auto* camera_manager = new CameraManager (*camera);
	camera_manager->add_camera (*camera);
	camera_manager->set_active_camera (camera->name);
	const Camera* active_camera = camera_manager->get_active_camera ();

	const glm::vec3 before = active_camera->transform.rotation;

	constexpr MouseInput mouse{10.0f, -5.0f};
	camera_manager->update_camera_look (&mouse);

	const glm::vec3 after = active_camera->transform.rotation;

	EXPECT_FALSE (glm::all (glm::equal (before, after)));
}
