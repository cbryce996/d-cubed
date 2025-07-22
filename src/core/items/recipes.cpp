#include "recipes.h"

#include <unordered_map>
#include <vector>

#include "items.h"

void register_item_recipe(
	const size_t id,
	const float crafting_time,
	const std::unordered_map<size_t, int>& inputs,
	const size_t output,
	const float output_amount
) {
	item_recipes.ids.push_back(id);
	item_recipes.crafting_times.push_back(crafting_time);

	item_recipe_inputs.ids.push_back(id);
	item_recipe_inputs.inputs.push_back(inputs);
	item_recipe_outputs.outputs.push_back(output);

	item_recipe_outputs.ids.push_back(id);
	item_recipe_outputs.output_amounts.push_back(output_amount);
}

void initialize_item_recipes() {
	register_item_recipe(
		1,
		10.0f,
		{{5, 5}},
		6,
		1
	);
}
