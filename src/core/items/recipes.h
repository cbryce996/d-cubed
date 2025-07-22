#ifndef ITEM_RECIPE_H
#define ITEM_RECIPE_H

#include <functional>
#include <unordered_map>
#include <vector>

struct ItemRecipes {
	std::vector<size_t> ids;
	std::vector<float> crafting_times;
};
inline ItemRecipes item_recipes;

struct ItemRecipeInputs {
	std::vector<size_t> ids;
	std::vector<std::unordered_map<size_t, int>> inputs;
};
inline ItemRecipeInputs item_recipe_inputs;

struct ItemRecipeOutputs {
	std::vector<size_t> ids;
	std::vector<size_t> outputs;
	std::vector<float> output_amounts;
};
inline ItemRecipeOutputs item_recipe_outputs;

void register_item_recipe(
	size_t id,
	float crafting_time,
	std::unordered_map<size_t, int>& inputs,
	size_t output,
	float output_amount
);

void initialize_item_recipes();

#endif	// ITEM_RECIPE_H
