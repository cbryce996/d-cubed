#include "static.h"

#include "assets/mesh/mesh.h"

StaticEntity::StaticEntity (
	std::string name, MeshInstance* mesh, MaterialInstance* material,
	const Transform& transform, const Transform& world_transform
)
	: IEntity (std::move (name), mesh, material, transform, world_transform) {}

void StaticEntity::on_load () { MeshTransfer::to_gpu (*mesh); }

void StaticEntity::on_unload () {}

void StaticEntity::update (float dt_ms, float sim_time_ms) {}
