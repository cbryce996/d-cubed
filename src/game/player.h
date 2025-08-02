#ifndef PLAYER_H
#define PLAYER_H

#include <SDL3/SDL.h>

#include "../engine/inputs.h"

class Player {
   public:
	Player();

	[[nodiscard]] SDL_FRect get_rect() const;
	void update(
		const InputManager& input,
		float delta_time
	);
	void render(SDL_Renderer* renderer) const;

   private:
	SDL_FRect rect;
	float speed = 200.0f;
};

#endif	// PLAYER_H
