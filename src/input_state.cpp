#include "input_state.h"

static Input_State g_input_state;

void Input_State::reset() {
	ch::mem_zero(keys_pressed, sizeof(keys_pressed));
	ch::mem_zero(mb_pressed, sizeof(mb_pressed));
}

void Input_State::process_input() {
	g_input_state.reset();
	ch::poll_events();

	ch::Vector2 mouse_pos;
	if (g_input_state.window && g_input_state.window->get_mouse_position(&mouse_pos)) {
		g_input_state.current_mouse_position = ch::Vector2((f32)mouse_pos.ux, (f32)mouse_pos.uy);
	} else {
		g_input_state.current_mouse_position = -1.f;
	}
}

const Input_State& Input_State::get() {
	return g_input_state;
}

void Input_State::bind(ch::Window* window) {
	g_input_state.window = window;

	window->on_exit_requested = [](const ch::Window& window) {
		g_input_state.exit_requested = true;
	};

	window->on_key_pressed = [](const ch::Window& window, u8 key) {
		if (!g_input_state.keys_down[key]) {
			g_input_state.keys_pressed[key] = true;
		}
		g_input_state.keys_down[key] = true;
	};

	window->on_key_released = [](const ch::Window& window, u8 key) {
		g_input_state.keys_down[key] = false;
	};

	window->on_mouse_button_down = [](const ch::Window& window, u8 mouse_button) {
		if (!g_input_state.mb_down[mouse_button]) {
			g_input_state.mb_pressed[mouse_button] = true;
		}
		g_input_state.mb_down[mouse_button] = true;
	};

	window->on_mouse_button_up = [](const ch::Window& window, u8 mouse_button) {
		g_input_state.mb_down[mouse_button] = false;
	};
}