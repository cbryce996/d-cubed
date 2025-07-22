#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>

#include "../engine/inputs.h"

class Player {
   public:
	Player();

	[[nodiscard]] SDL_Rect get_rect() const;
	void update(
		const InputManager& input,
		float delta_time
	);
	void render(SDL_Renderer* renderer) const;

   private:
	SDL_Rect rect;
	float speed = 200.0f;
};

#endif	// PLAYER_H
