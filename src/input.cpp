#include "input.h"
#include "game.h"

ch::Vector2 current_mouse_position;
bool exit_requested = false;
bool keys_down[255];
bool keys_pressed[255];

bool mb_down[3];
bool mb_pressed[3];

void init_input() {
	the_window.on_exit_requested = [](const ch::Window& window) {
		exit_requested = true;
	};

	the_window.on_key_pressed = [](const ch::Window& window, u8 key) {
		if (!keys_down[key]) {
			keys_pressed[key] = true;
		}
		keys_down[key] = true;
	};

	the_window.on_key_released = [](const ch::Window& window, u8 key) {
		keys_down[key] = false;
	};

	the_window.on_mouse_button_down = [](const ch::Window& window, u8 mouse_button) {
		if (!mb_down[mouse_button]) {
			mb_pressed[mouse_button] = true;
		}
		mb_down[mouse_button] = true;
	};

	the_window.on_mouse_button_up = [](const ch::Window& window, u8 mouse_button) {
		mb_down[mouse_button] = false;
	};
}

void process_input() {
	ch::mem_zero(keys_pressed, sizeof(keys_pressed));
	ch::mem_zero(mb_pressed, sizeof(mb_pressed));

	ch::poll_events();

	ch::Vector2 mouse_pos;
	if (the_window.get_mouse_position(&mouse_pos)) {
		current_mouse_position = ch::Vector2((f32)mouse_pos.ux, (f32)mouse_pos.uy);
	}
	else {
		current_mouse_position = -1.f;
	}

}

bool is_key_down(u8 key) {
	return keys_down[key];
}

bool was_key_pressed(u8 key) {
	return keys_pressed[key];
}

bool is_mouse_button_down(u8 mouse_button) {
	return mb_down[mouse_button];
}

bool was_mouse_button_pressed(u8 mouse_button) {
	return mb_pressed[mouse_button];
}

bool is_exit_requested() {
	return exit_requested;
}