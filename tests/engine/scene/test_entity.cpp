#include "entity/entity.h"
#include "utils.h"

#include <glm/glm.hpp>
#include <gtest/gtest.h>
#include <memory>

static glm::vec3 extract_translation (const glm::mat4& m) {
	return glm::vec3 (m[3]);
}

class TestEntity final : public IEntity {
  public:
	bool updated = false;

	explicit TestEntity (const std::string& name, const Transform& local = {})
		: IEntity (name, nullptr, nullptr, local, Transform{}) {}

	void update (float, float) override { updated = true; }
};

class EntityTest : public ::testing::Test {
  protected:
	std::unique_ptr<TestEntity> parent;
	std::unique_ptr<TestEntity> child;
	std::unique_ptr<TestEntity> grandchild;

	void SetUp () override {
		parent = std::make_unique<TestEntity> ("parent");
		child = std::make_unique<TestEntity> ("child");
		grandchild = std::make_unique<TestEntity> ("grandchild");
	}
};

TEST_F (EntityTest, ConstructorInitializesFieldsCorrectly) {
	EXPECT_EQ (parent->name, "parent");
	EXPECT_EQ (parent->mesh, nullptr);
	EXPECT_EQ (parent->material, nullptr);
}

TEST_F (EntityTest, SetParentAssignsParentPointer) {
	child->set_parent (parent.get ());
	EXPECT_EQ (child->parent, parent.get ());
}

TEST_F (EntityTest, SetParentAddsChildToParentChildrenList) {
	child->set_parent (parent.get ());

	ASSERT_EQ (parent->children.size (), 1);
	EXPECT_EQ (parent->children[0], child.get ());
}

TEST_F (EntityTest, AddChildSetsChildParent) {
	parent->add_child (child.get ());
	EXPECT_EQ (child->parent, parent.get ());
}

TEST_F (EntityTest, AddChildAddsToChildrenList) {
	parent->add_child (child.get ());

	ASSERT_EQ (parent->children.size (), 1);
	EXPECT_EQ (parent->children[0], child.get ());
}

TEST_F (EntityTest, SetParentAndAddChildProduceSameRelationship) {
	child->set_parent (parent.get ());

	EXPECT_EQ (child->parent, parent.get ());
	ASSERT_EQ (parent->children.size (), 1);
	EXPECT_EQ (parent->children[0], child.get ());
}

TEST_F (EntityTest, UpdateWorldTransformWithoutParentMatchesLocal) {
	parent->transform.position = {3, 4, 5};
	parent->transform.rotation = {0, 0, 0};
	parent->transform.scale = {1, 1, 1};

	parent->update_world_transform ();

	const glm::vec3 world_pos = extract_translation (parent->world_matrix);
	EXPECT_FLOAT_EQ (world_pos.x, 3.0f);
	EXPECT_FLOAT_EQ (world_pos.y, 4.0f);
	EXPECT_FLOAT_EQ (world_pos.z, 5.0f);
}

TEST_F (EntityTest, UpdateWorldTransformWithParentCombinesTransforms) {
	parent->transform.position = {10, 0, 0};
	child->transform.position = {5, 0, 0};

	child->set_parent (parent.get ());

	parent->update_world_transform ();

	const glm::vec3 child_world = extract_translation (child->world_matrix);
	EXPECT_FLOAT_EQ (child_world.x, 15.0f);
	EXPECT_FLOAT_EQ (child_world.y, 0.0f);
	EXPECT_FLOAT_EQ (child_world.z, 0.0f);
}

TEST_F (EntityTest, MultiLevelHierarchyWorldTransformPropagation) {
	parent->transform.position = {10, 0, 0};
	child->transform.position = {5, 0, 0};
	grandchild->transform.position = {2, 0, 0};

	child->set_parent (parent.get ());
	grandchild->set_parent (child.get ());

	parent->update_world_transform ();

	const glm::vec3 g_world = extract_translation (grandchild->world_matrix);
	EXPECT_FLOAT_EQ (g_world.x, 17.0f);
	EXPECT_FLOAT_EQ (g_world.y, 0.0f);
	EXPECT_FLOAT_EQ (g_world.z, 0.0f);
}

TEST_F (EntityTest, UpdateWorldTransformIsDeterministic) {
	parent->transform.position = {1, 2, 3};

	parent->update_world_transform ();
	const glm::vec3 first = extract_translation (parent->world_matrix);

	parent->update_world_transform ();
	const glm::vec3 second = extract_translation (parent->world_matrix);

	EXPECT_FLOAT_EQ (first.x, second.x);
	EXPECT_FLOAT_EQ (first.y, second.y);
	EXPECT_FLOAT_EQ (first.z, second.z);
}

TEST_F (EntityTest, ChildUpdateDoesNotMutateParentWorldMatrix) {
	parent->transform.position = {10, 0, 0};
	child->transform.position = {5, 0, 0};

	child->set_parent (parent.get ());

	parent->update_world_transform ();
	const glm::vec3 parent_before = extract_translation (parent->world_matrix);

	child->update_world_transform ();

	const glm::vec3 parent_after = extract_translation (parent->world_matrix);

	EXPECT_FLOAT_EQ (parent_before.x, parent_after.x);
	EXPECT_FLOAT_EQ (parent_before.y, parent_after.y);
	EXPECT_FLOAT_EQ (parent_before.z, parent_after.z);
}
