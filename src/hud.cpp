#include "hud.h"
#include "draw.h"
#include "game.h"
#include "input.h"

static bool show_debug_memory = false;

void tick_hud(f32 dt) {
	if (was_key_pressed(CH_KEY_F1)) {
		show_debug_memory = !show_debug_memory;
	}
}

void draw_hud() {
	render_right_handed();

	if (show_debug_memory) {
		ch::String bytes_string;
		defer(bytes_string.free());
		ch::bytes_to_string(ch::get_total_allocated(), &bytes_string);

		tchar buffer[1024];
		ch::sprintf(buffer, CH_TEXT("Num Allocations: %llu\nTotal Allocated: %.*s"), ch::get_num_allocations(), bytes_string.count, bytes_string.data);
		draw_string(buffer, 5.f, -FONT_SIZE, ch::white, font);	
	}
}