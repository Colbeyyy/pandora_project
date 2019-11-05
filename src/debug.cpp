#include "debug.h"
#include "input.h"
#include "tile.h"
#include "world.h"
#include "gui.h"
#include "font.h"
#include "console.h"

bool show_tile_grid = false;
bool show_transform_origin = false;
bool show_collider_debug = false;
bool show_debug_memory = false;
bool show_fps = false;


static void debug_input(void* owner, Input_Event* event) {
	const char c = event->c;

	switch (c) {
		case CH_KEY_F1:
			show_debug_memory = !show_debug_memory;
			break;
		case CH_KEY_F2:
			show_tile_grid = !show_tile_grid;
			break;
		case CH_KEY_F3: 
			show_transform_origin = !show_transform_origin;
			break;
		case CH_KEY_F4:
			show_collider_debug = !show_collider_debug;
			break;
		case CH_KEY_F5:
			show_fps = !show_fps;
			break;
	}
}

void init_debug() {
	Event_Listener el(nullptr, debug_input, ET_Key_Pressed);
	bind_event_listener(el);
}

void tick_debug(f32 dt) {
	Vertical_Layout layout(0.f, 0.f, FONT_SIZE + 2.f);

	if (show_debug_memory) {
		{
			char buffer[128];
			ch::sprintf(buffer, "Num Allocations: %llu", ch::get_num_allocations());
			gui_text(buffer, layout.at_x, layout.at_y, ch::white);
			layout.row();
		}
		{
			ch::String bytes_string;
			defer(bytes_string.free());
			ch::bytes_to_string(ch::get_total_allocated(), &bytes_string);

			char buffer[128];
			ch::sprintf(buffer, "Total Allocated: %.*s", bytes_string.count, bytes_string.data);
			gui_text(buffer, layout.at_x, layout.at_y, ch::white);
			layout.row(); 
		}
	}

	if (show_fps) {
		char buffer[1024];
		ch::sprintf(buffer, "FPS: %i", (s32)(1.f / dt));
		gui_text(buffer, layout.at_x, layout.at_y, ch::white);
		layout.row();
	}
}

bool toggle_fps(const ch::String& params) {
	if (params) {
		console_log("toggle_fps should not have any params");
		return false;
	}

	show_fps = !show_fps;
	return true;
}
