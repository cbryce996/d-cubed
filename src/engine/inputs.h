#ifndef INPUTS_H
#define INPUTS_H

#include <SDL3/SDL.h>

class InputManager {
	public:
		void poll();

		[[nodiscard]] const bool* get_key_states() const;
		[[nodiscard]] bool is_key_down(SDL_Scancode key) const;
		[[nodiscard]] bool is_key_pressed(SDL_Scancode key) const;
		[[nodiscard]] bool is_mouse_pressed(Uint32 button) const;
		[[nodiscard]] SDL_Point get_mouse_position() const;

	private:
		const bool *key_states = nullptr;
		Uint8 prev_key_states[SDL_SCANCODE_COUNT]{};

		int mouse_x = 0;
		int mouse_y = 0;
		Uint32 mouse_button_state = 0;
		Uint32 prev_mouse_button_state = 0;
	};


#endif	// INPUTS_H
