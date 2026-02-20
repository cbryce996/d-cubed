#ifndef INPUTS_H
#define INPUTS_H

#include <SDL3/SDL.h>

struct MouseInput {
	float dx = 0.0f;
	float dy = 0.0f;
	Uint32 button_state = 0;
};

struct KeyboardInput {
	bool keys[SDL_SCANCODE_COUNT]{};
};

class InputManager {
  public:
	InputManager ();
	~InputManager ();

	void poll ();

	[[nodiscard]] const MouseInput& get_mouse_input () const;
	[[nodiscard]] const KeyboardInput& get_keyboard_input () const;

  private:
	MouseInput mouse_input;
	KeyboardInput keyboard_input;
};

#endif // INPUTS_H
