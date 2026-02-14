#include "entity.h"

#include "utils.h"

IEntity::IEntity (
	std::string name, MeshInstance* mesh, MaterialInstance* material,
	const Transform& transform, const Transform& world_transform
)
	: name (std::move (name)), transform (transform),
	  world_transform (world_transform), mesh (mesh), material (material) {}

IEntity::~IEntity () = default;

void IEntity::set_parent (IEntity* in_entity) {
	parent = in_entity;
	if (in_entity) {
		in_entity->children.push_back (this);
	}
}

void IEntity::add_child (IEntity* in_entity) {
	if (!in_entity)
		return;
	in_entity->parent = this;
	children.push_back (in_entity);
}

void IEntity::update_world_transform () {
	const glm::mat4 local = transform.to_mat4 ();

	if (parent) {
		world_matrix = parent->world_matrix * local;
	} else {
		world_matrix = local;
	}

	for (IEntity* child : children) {
		child->update_world_transform ();
	}
}
