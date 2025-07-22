#include "inputs.h"

#include <SDL2/SDL.h>

void InputManager::poll() {
	prev_mouse_button_state = mouse_button_state;
	mouse_button_state = SDL_GetMouseState(&mouse_x, &mouse_y);
	SDL_PumpEvents();
	key_states = SDL_GetKeyboardState(nullptr);
}

bool InputManager::is_mouse_pressed(Uint8 button) const {
	return (mouse_button_state & SDL_BUTTON(button)) &&
		   !(prev_mouse_button_state & SDL_BUTTON(button));
}

bool InputManager::is_key_down(const SDL_Scancode key) const {
	return key_states && key_states[key];
}

SDL_Point InputManager::get_mouse_position() const {
	return SDL_Point{mouse_x, mouse_y};
}