#include "entity/entity.h"
#include "utils.h"

#include <gtest/gtest.h>
#include <memory>

class TestEntity final : public IEntity {
  public:
	bool updated = false;

	explicit TestEntity (
		const std::string& name, const Transform& local = {},
		const Transform& world = {}
	)
		: IEntity (name, nullptr, nullptr, local, world) {}

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

TEST_F (EntityTest, UpdateWorldTransformWithoutParentCopiesLocalTransform) {
	parent->transform.position = {3, 4, 5};

	parent->update_world_transform ();

	EXPECT_EQ (parent->world_transform.position.x, 3);
	EXPECT_EQ (parent->world_transform.position.y, 4);
	EXPECT_EQ (parent->world_transform.position.z, 5);
}

TEST_F (EntityTest, UpdateWorldTransformWithParentCombinesTransforms) {
	parent->transform.position = {10, 0, 0};
	child->transform.position = {5, 0, 0};

	child->set_parent (parent.get ());

	parent->update_world_transform ();
	child->update_world_transform ();

	EXPECT_EQ (child->world_transform.position.x, 15);
	EXPECT_EQ (child->world_transform.position.y, 0);
	EXPECT_EQ (child->world_transform.position.z, 0);
}

TEST_F (EntityTest, MultiLevelHierarchyWorldTransformPropagation) {
	parent->transform.position = {10, 0, 0};
	child->transform.position = {5, 0, 0};
	grandchild->transform.position = {2, 0, 0};

	child->set_parent (parent.get ());
	grandchild->set_parent (child.get ());

	parent->update_world_transform ();
	child->update_world_transform ();
	grandchild->update_world_transform ();

	EXPECT_EQ (grandchild->world_transform.position.x, 17);
	EXPECT_EQ (grandchild->world_transform.position.y, 0);
	EXPECT_EQ (grandchild->world_transform.position.z, 0);
}

TEST_F (EntityTest, UpdateWorldTransformIsDeterministic) {
	parent->transform.position = {1, 2, 3};

	parent->update_world_transform ();
	const auto first = parent->world_transform.position;

	parent->update_world_transform ();
	const auto second = parent->world_transform.position;

	EXPECT_EQ (first.x, second.x);
	EXPECT_EQ (first.y, second.y);
	EXPECT_EQ (first.z, second.z);
}

TEST_F (EntityTest, ChildUpdateDoesNotMutateParentWorldTransform) {
	parent->transform.position = {10, 0, 0};
	child->transform.position = {5, 0, 0};

	child->set_parent (parent.get ());

	parent->update_world_transform ();
	const auto parent_before = parent->world_transform.position;

	child->update_world_transform ();

	const auto parent_after = parent->world_transform.position;

	EXPECT_EQ (parent_before.x, parent_after.x);
	EXPECT_EQ (parent_before.y, parent_after.y);
	EXPECT_EQ (parent_before.z, parent_after.z);
}
