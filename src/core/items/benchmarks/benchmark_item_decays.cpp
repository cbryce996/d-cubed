#include <chrono>
#include <iostream>

#include "../item_types.h"
#include "../items.h"

constexpr uint32_t num_items = 1'000'000;

int main() {
	initialize_item_types();  // Populate global
							  // item_types

	std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

	// === Load & Init ===
	start = std::chrono::high_resolution_clock::now();

	for (uint32_t i = 0; i < num_items; ++i) {
		register_item(
			ItemId{i},
			ItemTypeId{1},
			1.0f,  // age
			0.0f,  // initial decay
			0.0f   // initial value
		);
	}

	end = std::chrono::high_resolution_clock::now();
	const auto load_time =
		std::chrono::duration<double, std::milli>(end - start).count();
	std::cout << "[Load & Init] " << load_time << " ms\n";

	// === Compute (Decay Calculation) ===
	start = std::chrono::high_resolution_clock::now();
	calculate_item_decays(item_decays, items);
	end = std::chrono::high_resolution_clock::now();
	const auto compute_time =
		std::chrono::duration<double, std::milli>(end - start).count();
	std::cout << "[Decay Compute] " << compute_time << " ms\n";

	// === Summary
	std::cout << "\n=== Summary ===\n";
	std::cout << "Total: " << (load_time + compute_time) << " ms\n";

	const size_t sample_index = 137;

	if (sample_index < num_items) {
		const auto& id = items.ids[sample_index];
		const auto& type_id = items.type_ids[sample_index];
		const float age = item_decays.ages[sample_index];
		const float decay = item_decays.decays[sample_index];
		const float rate = item_type_decays.decay_rates[type_id.value];

		std::cout << "\n=== Sample Item ===\n";
		std::cout << "ID: " << id.value << "\n";
		std::cout << "Type ID: " << type_id.value << "\n";
		std::cout << "Decay Rate: " << rate << "\n";
		std::cout << "Age: " << age << "\n";
		std::cout << "Computed Decay: " << decay << "\n";
	}

	return 0;
}
