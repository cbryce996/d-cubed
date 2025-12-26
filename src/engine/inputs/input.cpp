#include "input.h"

#include <SDL3/SDL.h>
#include <cstring>

InputManager::InputManager() = default;

InputManager::~InputManager() = default;

void InputManager::poll() {
	SDL_PumpEvents();
	const bool* state = SDL_GetKeyboardState(nullptr);
	memcpy(keyboard_input.keys, state, SDL_SCANCODE_COUNT);

	float x, y;
	mouse_input.button_state = SDL_GetMouseState(&x, &y);

	float dx, dy;
	SDL_GetRelativeMouseState(&dx, &dy);
	mouse_input.dx = dx;
	mouse_input.dy = dy;
}

const MouseInput& InputManager::get_mouse_input() const {
	return mouse_input;
}

const KeyboardInput& InputManager::get_keyboard_input() const {
	return keyboard_input;
}
