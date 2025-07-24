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

void Player::update(const InputManager& input, float delta_time) {
	float dt = delta_time / 1000.0f;

	float dx = 0.0f;
	float dy = 0.0f;

	if (input.is_key_down(SDL_SCANCODE_W))
		dy -= 1.0f;
	if (input.is_key_down(SDL_SCANCODE_S))
		dy += 1.0f;
	if (input.is_key_down(SDL_SCANCODE_A))
		dx -= 1.0f;
	if (input.is_key_down(SDL_SCANCODE_D))
		dx += 1.0f;

	if (dx != 0.0f || dy != 0.0f) {
		const float length = std::sqrt(dx * dx + dy * dy);
		dx /= length;
		dy /= length;
	}

	rect.x += dx * speed * dt;
	rect.y += dy * speed * dt;
}

void Player::render(SDL_Renderer* renderer) const {
	SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
	SDL_RenderFillRect(renderer, &rect);
}
