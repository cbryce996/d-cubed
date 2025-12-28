#ifndef CRAFTING_H
#define CRAFTING_H

#include <functional>
#include <vector>

struct ItemCrafting {
	std::vector<size_t> ids;
	std::vector<size_t> recipe_ids;
	std::vector<float> progress;
};
inline ItemCrafting item_crafting;

void register_item_crafting (size_t id, size_t recipe_id, float progress);

#endif // CRAFTING_H
