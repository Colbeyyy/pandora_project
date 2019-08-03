#pragma once

#include <ch_stl/input.h>
#include <ch_stl/math.h>
#include <ch_stl/window.h>

struct Input_State {
	ch::Vector2 current_mouse_position;
	bool exit_requested = false;
	bool keys_down[255];
	bool keys_pressed[255];

	bool mb_down[3];
	bool mb_pressed[3];

	const ch::Window* window;

	void reset();

	CH_FORCEINLINE bool is_key_down(u8 key) const { return keys_down[key]; }
	CH_FORCEINLINE bool was_key_pressed(u8 key) const { return keys_pressed[key]; }

	CH_FORCEINLINE bool is_mouse_button_down(u8 mouse_button) const { return mb_down[mouse_button]; }
	CH_FORCEINLINE bool was_mouse_button_pressed(u8 mouse_button) const { return mb_pressed[mouse_button]; }

	static const Input_State& get();
	static void process_input();
	static void bind(ch::Window* window);
};