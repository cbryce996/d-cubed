#ifndef ITEM_TYPES_H
#define ITEM_TYPES_H

#include <functional>
#include <unordered_map>
#include <vector>

struct ItemTypes {
	std::vector<size_t> ids;
	std::vector<std::optional<size_t>> base_type_ids;
	std::vector<std::string> names;
};
inline ItemTypes item_types;

struct ItemTypeDecays {
	std::vector<size_t> ids;
	std::vector<float> decay_rates;
	std::vector<std::vector<std::string>> decay_labels;
};
inline ItemTypeDecays item_type_decays;

struct ItemTypeValues {
	std::vector<size_t> ids;
	std::vector<float> base_values;
	std::vector<float> min_values;
};
inline ItemTypeValues item_type_values;

void initialize_item_types ();

#endif // ITEM_TYPES_H
