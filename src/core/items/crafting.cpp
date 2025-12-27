#include "crafting.h"

void register_item_crafting (
	const size_t id, const size_t recipe_id, const float progress
) {
	item_crafting.ids.push_back (id);
	item_crafting.recipe_ids.push_back (recipe_id);
	item_crafting.progress.push_back (progress);
}
