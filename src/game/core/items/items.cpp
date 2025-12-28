#include "items.h"

void register_item (
	const size_t id, const size_t type_id, const float age, const float decay,
	const float value, const int x, const int y
) {
	items.ids.push_back (id);
	items.type_ids.push_back (type_id);
	items.x.push_back (x);
	items.y.push_back (y);

	item_decays.ids.push_back (id);
	item_decays.ages.push_back (age);
	item_decays.decays.push_back (decay);

	item_values.ids.push_back (id);
	item_values.values.push_back (value);
}
