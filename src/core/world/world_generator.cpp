#include "world_generator.h"

#include <random>

#include "world_geometry.h"

WorldGenerator::WorldGenerator(const VoronoiWorldConfig& cfg) : config(cfg) {}

SDL_Color WorldGenerator::generate_color(const int index) {
	std::mt19937 rng(index);
	std::uniform_int_distribution<int> dist(50, 200);
	return {(Uint8)dist(rng), (Uint8)dist(rng), (Uint8)dist(rng), 255};
}

void WorldGenerator::generate() {
	std::vector<Vec2> points;
	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> distX(0, config.width);
	std::uniform_real_distribution<float> distY(0, config.height);

	for (int i = 0; i < config.num_points; ++i) {
		points.push_back({distX(rng), distY(rng)});
	}

	for (int i = 0; i < config.lloyd_iterations; ++i) {
		std::vector<Vec2> new_points(points.size(), {0, 0});
		std::vector<int> counts(points.size(), 0);

		for (int y = 0; y < config.height; y += 2) {
			for (int x = 0; x < config.width; x += 2) {
				Vec2 sample = {(float)x, (float)y};
				int closest = 0;
				float min_dist = sample.distance_squared(points[0]);

				for (int j = 1; j < points.size(); ++j) {
					float d = sample.distance_squared(points[j]);
					if (d < min_dist) {
						min_dist = d;
						closest = j;
					}
				}

				new_points[closest] = new_points[closest] + sample;
				counts[closest]++;
			}
		}

		for (int j = 0; j < points.size(); ++j) {
			if (counts[j] > 0)
				new_points[j] = new_points[j] / (float)counts[j];
			else
				new_points[j] = points[j];
		}

		points = new_points;
	}

	regions = WorldGeometry::generate_regions(points, config.width, config.height);

	region_colors.clear();
	for (int i = 0; i < config.num_points; ++i) {
		region_colors.push_back(generate_color(i));
	}
}

void WorldGenerator::render(SDL_Renderer* renderer) {
	for (int i = 0; i < regions.size(); ++i) {
		const auto& region = regions[i];
		const auto& color = region_colors[i];

		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

		for (size_t j = 0; j < region.polygon.size(); ++j) {
			const auto& a = region.polygon[j];
			const auto& b = region.polygon[(j + 1) % region.polygon.size()];
			SDL_RenderDrawLine(
				renderer,
				static_cast<int>(a.x),
				static_cast<int>(a.y),
				static_cast<int>(b.x),
				static_cast<int>(b.y)
			);
		}
	}

	// Draw centers
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	for (const auto& region : regions) {
		SDL_Rect dot = {static_cast<int>(region.center.x) - 2, static_cast<int>(region.center.y) - 2, 4, 4};
		SDL_RenderFillRect(renderer, &dot);
	}
}
