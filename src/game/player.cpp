#include "player.h"
#include <cmath>

Player::Player() {
	rect.x = 100.0f;
	rect.y = 100.0f;
	rect.w = 50.0f;
	rect.h = 50.0f;
}

SDL_FRect Player::get_rect() const {
	return rect;
}

void Player::update(
	const InputManager& input,
	float delta_time
) {
	// TODOs
}

void Player::render(SDL_Renderer* renderer) const {
	SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
	SDL_RenderFillRect(renderer, &rect);
}
