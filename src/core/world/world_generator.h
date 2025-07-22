#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <SDL2/SDL.h>

#include <vector>

#include "../world/world_geometry.h"

struct VoronoiWorldConfig {
	int width = 800;
	int height = 600;
	int num_points = 100;
	int lloyd_iterations = 5;
};

class WorldGenerator {
   public:
	explicit WorldGenerator(const VoronoiWorldConfig& cfg);

	void generate();
	void render(SDL_Renderer* renderer);

	std::vector<Region> regions;

   private:
	VoronoiWorldConfig config;
	SDL_Color generate_color(int index);
	std::vector<SDL_Color> region_colors;
};

#endif
