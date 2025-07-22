#include <unordered_map>

#include "agent.h"

std::unordered_map<AgentId, AgentType> ItemTypes;

void register_item_type(
	const ItemTypeId& id,
	ItemType type
) {
	ItemTypes[id] = std::move(type);
}

void initialize_item_types();