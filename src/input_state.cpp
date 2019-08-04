#include "input_state.h"

Input_State input_state;

void Input_State::reset() {
	ch::mem_zero(keys_pressed, sizeof(keys_pressed));
	ch::mem_zero(mb_pressed, sizeof(mb_pressed));
}

void Input_State::process_input() {
	ch::Vector2 mouse_pos;
	if (input_state.window && input_state.window->get_mouse_position(&mouse_pos)) {
		input_state.current_mouse_position = ch::Vector2((f32)mouse_pos.ux, (f32)mouse_pos.uy);
	} else {
		input_state.current_mouse_position = -1.f;
	}
}
void Input_State::bind(ch::Window* window) {
	window = window;

	window->on_exit_requested = [](const ch::Window& window) {
		input_state.exit_requested = true;
	};

	window->on_key_pressed = [](const ch::Window& window, u8 key) {
		if (!input_state.keys_down[key]) {
			input_state.keys_pressed[key] = true;
		}
		input_state.keys_down[key] = true;
	};

	window->on_key_released = [](const ch::Window& window, u8 key) {
		input_state.keys_down[key] = false;
	};

	window->on_mouse_button_down = [](const ch::Window& window, u8 mouse_button) {
		if (!input_state.mb_down[mouse_button]) {
			input_state.mb_pressed[mouse_button] = true;
		}
		input_state.mb_down[mouse_button] = true;
	};

	window->on_mouse_button_up = [](const ch::Window& window, u8 mouse_button) {
		input_state.mb_down[mouse_button] = false;
	};
}