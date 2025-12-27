#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../src/engine/cameras/camera.h"

#include <SDL3/SDL.h>

class CameraTest : public ::testing::Test {
   protected:
	void SetUp() override {
		camera->name = "test_cam";
		camera->transform.position = glm::vec3(0.0f);
		camera->transform.rotation = glm::quat(1, 0, 0, 0);
		camera->move_speed = 1.0f;
		camera->look_sensitivity = 1.0f;
	}

	Camera* camera = new Camera();
};

TEST_F(
	CameraTest,
	RetrieveCamera
) {
	CameraManager* camera_manager = new CameraManager();
	camera_manager->add_camera(*camera);
	Camera* retrieved_camera = camera_manager->get_camera("test_cam");
	ASSERT_NE(retrieved_camera, nullptr) << "Expected camera to not be null";
	EXPECT_EQ(retrieved_camera->name, "test_cam") << "Expected camera to have "
													 "name";
}

TEST_F(
	CameraTest,
	ReturnsNullForMissingCamera
) {
	CameraManager* camera_manager = new CameraManager();
	Camera* cam = camera_manager->get_camera("null_cam");
	ASSERT_EQ(cam, nullptr) << "Expected camera to be null";
}

TEST_F(
	CameraTest,
	GetActiveCamera
) {
	CameraManager* camera_manager = new CameraManager();
	camera_manager->add_camera(*camera);
	camera_manager->set_active_camera(*camera);
	Camera* active_camera = camera_manager->get_active_camera();
	ASSERT_NE(active_camera, nullptr) << "Expected camera to not be null";
	EXPECT_EQ(active_camera->name, "test_cam") << "Expected camera to have "
												  "name";
}

TEST_F(
	CameraTest,
	UpdatePositionForward
) {
	CameraManager* camera_manager = new CameraManager();
	camera_manager->add_camera(*camera);
	camera_manager->set_active_camera(*camera);
	Camera* active_camera = camera_manager->get_active_camera();

	glm::vec3 start_pos = active_camera->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_W] = true;

	camera_manager->update_camera_position(1.0f, keys);

	EXPECT_LT(active_camera->transform.position.z, start_pos.z) << "Expected "
																   "camera "
																   "position "
																   "to move "
																   "backward";
}

TEST_F(
	CameraTest,
	UpdatePositionBackward
) {
	CameraManager* camera_manager = new CameraManager();
	camera_manager->add_camera(*camera);
	camera_manager->set_active_camera(*camera);
	Camera* active_camera = camera_manager->get_active_camera();

	glm::vec3 start_pos = active_camera->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_S] = true;

	camera_manager->update_camera_position(1.0f, keys);

	EXPECT_GT(active_camera->transform.position.z, start_pos.z) << "Expected "
																   "camera "
																   "position "
																   "to move "
																   "forward";
}

TEST_F(
	CameraTest,
	UpdatePositionRight
) {
	CameraManager* camera_manager = new CameraManager();
	camera_manager->add_camera(*camera);
	camera_manager->set_active_camera(*camera);
	Camera* active_camera = camera_manager->get_active_camera();

	glm::vec3 start_pos = active_camera->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_D] = true;

	camera_manager->update_camera_position(1.0f, keys);

	EXPECT_GT(active_camera->transform.position.x, start_pos.x) << "Expected "
																   "camera "
																   "position "
																   "to move "
																   "right";
}

TEST_F(
	CameraTest,
	UpdatePositionLeft
) {
	CameraManager* camera_manager = new CameraManager();
	camera_manager->add_camera(*camera);
	camera_manager->set_active_camera(*camera);
	Camera* active_camera = camera_manager->get_active_camera();

	glm::vec3 start_pos = active_camera->transform.position;

	bool keys[SDL_SCANCODE_COUNT + 1] = {false};
	keys[SDL_SCANCODE_A] = true;

	camera_manager->update_camera_position(1.0f, keys);

	EXPECT_LT(active_camera->transform.position.x, start_pos.x) << "Expected "
																   "camera "
																   "position "
																   "to move "
																   "left";
}

TEST_F(
	CameraTest,
	UpdateLookChangesRotation
) {
	CameraManager* camera_manager = new CameraManager();
	camera_manager->add_camera(*camera);
	camera_manager->set_active_camera(*camera);
	Camera* active_camera = camera_manager->get_active_camera();

	glm::quat before = active_camera->transform.rotation;

	MouseInput mouse{10.0f, -5.0f};

	camera_manager->update_camera_look(&mouse);
	glm::quat after = active_camera->transform.rotation;

	// The rotation should have changed
	EXPECT_FALSE(glm::all(glm::equal(before, after))) << "Expected camera "
														 "rotation to change";
}