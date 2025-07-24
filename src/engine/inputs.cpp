#include "inputs.h"

#include <SDL3/SDL.h>

void InputManager::poll() {
	prev_mouse_button_state = mouse_button_state;

	float x, y;
	mouse_button_state = SDL_GetMouseState(&x, &y);
	mouse_x = static_cast<int>(x);
	mouse_y = static_cast<int>(y);

	SDL_PumpEvents();  // Fetch input
	key_states = SDL_GetKeyboardState(nullptr);

	memcpy(prev_key_states, key_states, SDL_SCANCODE_COUNT);
}

const bool* InputManager::get_key_states() const {
	return key_states;
}

bool InputManager::is_mouse_pressed(const Uint32 button) const {
	return (mouse_button_state & button) &&
		   !(prev_mouse_button_state & button);
}

bool InputManager::is_key_down(SDL_Scancode key) const {
	return key_states[key];
}

bool InputManager::is_key_pressed(SDL_Scancode key) const {
	return key_states[key] && !prev_key_states[key];
}

SDL_Point InputManager::get_mouse_position() const {
	return SDL_Point{mouse_x, mouse_y};
}
