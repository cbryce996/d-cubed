#ifndef ITEMS_H
#define ITEMS_H

#include <functional>
#include <vector>

struct Items {
	std::vector<size_t> ids;
	std::vector<size_t> type_ids;
};
inline Items items;

struct ItemDecays {
	std::vector<size_t> ids;
	std::vector<float> ages;
	std::vector<float> decays;
};
inline ItemDecays item_decays;

struct ItemValues {
	std::vector<size_t> ids;
	std::vector<float> values;
};
inline ItemValues item_values;

void register_item(
	size_t id,
	size_t type_id,
	float age,
	float decay,
	float value
);

#endif	// ITEMS_H
