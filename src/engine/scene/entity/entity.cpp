#include "entity.h"

#include "utils.h"

IEntity::IEntity (std::string name) : name (std::move (name)) {}

IEntity::~IEntity () = default;