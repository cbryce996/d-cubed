#include "items.h"

void register_item(
	const size_t id,
	const size_t type_id,
	const float age,
	const float decay,
	const float value
) {
	items.ids.push_back(id);
	items.type_ids.push_back(type_id);

	item_decays.ids.push_back(id);
	item_decays.ages.push_back(age);
	item_decays.decays.push_back(decay);

	item_values.ids.push_back(id);
	item_values.values.push_back(value);
}