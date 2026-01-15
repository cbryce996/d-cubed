#include "object.h"

ISceneObject::ISceneObject (std::string name) : name (std::move (name)) {}

ISceneObject::~ISceneObject () = default;

void ISceneObject::collect_drawables (RenderState&) {
	// default: no drawables
}