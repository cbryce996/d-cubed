#include "entity/entity.h"
#include "render/render.h"
#include "scene.h"

#include <gtest/gtest.h>

class TestEntity final : public IEntity {
  public:
	bool loaded = false;
	bool unloaded = false;
	bool updated = false;

	explicit TestEntity (const std::string& name)
		: IEntity (name, nullptr, nullptr, Transform{}, Transform{}) {}

	void on_load () override { loaded = true; }

	void on_unload () override { unloaded = true; }

	void update (float, float) override { updated = true; }
};

class SceneTest : public ::testing::Test {
  protected:
	Scene scene;
};

TEST_F (SceneTest, AddEntityStoresEntity) {
	auto entity = std::make_unique<TestEntity> ("entity_1");

	scene.add_entity (std::move (entity));

	RenderState render_state;
	scene.collect_drawables (render_state);

	EXPECT_EQ (render_state.drawables.size (), 1);
}

TEST_F (SceneTest, OnLoadCallsEntityOnLoad) {
	auto entity = std::make_unique<TestEntity> ("entity_1");
	const TestEntity* raw = entity.get ();

	scene.add_entity (std::move (entity));
	scene.on_load ();

	EXPECT_TRUE (raw->loaded);
}

TEST_F (SceneTest, OnUnloadCallsEntityOnUnload) {
	auto entity = std::make_unique<TestEntity> ("entity_1");
	const TestEntity* raw = entity.get ();

	scene.add_entity (std::move (entity));
	scene.on_load ();
	scene.on_unload ();

	EXPECT_TRUE (raw->unloaded);
}

TEST_F (SceneTest, UpdateCallsEntityUpdate) {
	auto entity = std::make_unique<TestEntity> ("entity_1");
	const TestEntity* raw = entity.get ();

	scene.add_entity (std::move (entity));
	scene.on_load ();

	scene.update (16.0f, 100.0f);

	EXPECT_TRUE (raw->updated);
}

TEST_F (SceneTest, UpdateCallsWorldTransformUpdate) {
	auto entity = std::make_unique<TestEntity> ("entity_1");
	TestEntity* raw = entity.get ();

	raw->transform.position = {5, 0, 0};

	scene.add_entity (std::move (entity));
	scene.on_load ();

	scene.update (16.0f, 0.0f);

	EXPECT_EQ (raw->world_transform.position.x, 5);
}

TEST_F (SceneTest, DuplicateEntityNamesAreRejected) {
	auto entity_1 = std::make_unique<TestEntity> ("entity");
	auto entity_2 = std::make_unique<TestEntity> ("entity");

	scene.add_entity (std::move (entity_1));
	scene.add_entity (std::move (entity_2));

	RenderState render_state{};
	scene.collect_drawables (render_state);

	EXPECT_EQ (render_state.drawables.size (), 1);
}

TEST_F (SceneTest, AddEntityAfterSceneLoadCallsOnLoadImmediately) {
	scene.on_load ();

	auto entity = std::make_unique<TestEntity> ("late");
	const TestEntity* raw = entity.get ();

	scene.add_entity (std::move (entity));

	EXPECT_TRUE (raw->loaded);
}

TEST_F (SceneTest, CollectDrawablesCreatesDrawablePerEntity) {
	auto entity_1 = std::make_unique<TestEntity> ("entity_1");
	auto entity_2 = std::make_unique<TestEntity> ("entity_2");
	auto entity_3 = std::make_unique<TestEntity> ("entity_3");

	scene.add_entity (std::move (entity_1));
	scene.add_entity (std::move (entity_2));
	scene.add_entity (std::move (entity_3));

	RenderState render_state{};
	scene.collect_drawables (render_state);

	EXPECT_EQ (render_state.drawables.size (), 3);
}
