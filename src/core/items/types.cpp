#include "types.h"

#include <vector>

void register_item_type(
	const size_t id,
	const std::string& name,
	const std::optional<size_t> base_type,
	const float base_value,
	const float min_value_ratio,
	const float decay_rate,
	const std::vector<std::string>& decay_labels
) {
	item_types.ids.push_back(id);
	item_types.names.push_back(name);
	item_types.base_type_ids.push_back(base_type);

	item_type_values.ids.push_back(id);
	item_type_values.base_values.push_back(base_value);
	item_type_values.min_values.push_back(min_value_ratio);

	item_type_decays.ids.push_back(id);
	item_type_decays.decay_rates.push_back(decay_rate);
	item_type_decays.decay_labels.push_back(decay_labels);
}

void initialize_item_types() {
	register_item_type(1, "Fish", std::nullopt, 10.0f, 0.05f, 0.3f, {"Fresh", "Spoiling", "Rotten", "Disgusting"});

	register_item_type(2, "Fine Iron Sword", 14, 200.0f, 2.0f, 0.05f, {"Sharp", "Dull"});

	register_item_type(3, "Average Iron Sword", 14, 100.0f, 1.0f, 0.3f, {"Sharp", "Dull", "Rusted"});
}
