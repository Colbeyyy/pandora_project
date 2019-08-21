#pragma once

#include <ch_stl/math.h>
#include <ch_stl/input.h>

extern ch::Vector2 current_mouse_position;

void init_input();
void process_input();

bool is_key_down(u8 key);
bool was_key_pressed(u8 key);

bool is_mouse_button_down(u8 mouse_button);
bool was_mouse_button_pressed(u8 mouse_button);

bool is_exit_requested();