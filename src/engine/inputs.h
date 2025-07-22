#ifndef INPUTS_H
#define INPUTS_H

#include <SDL2/SDL.h>

class InputManager {
	public:
		void poll();

		[[nodiscard]] bool is_key_down(SDL_Scancode key) const;
		[[nodiscard]] bool is_mouse_pressed(Uint8 button) const;
		SDL_Point get_mouse_position() const;

	private:
		const Uint8* key_states = nullptr;
		Uint32 mouse_button_state = 0;
		Uint32 prev_mouse_button_state = 0;
		int mouse_x = 0;
		int mouse_y = 0;
};

#endif	// INPUTS_H
