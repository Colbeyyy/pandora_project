#include "debug.h"
#include "input.h"
#include "tile.h"
#include "world.h"

bool show_tile_grid = false;
bool show_transform_origin = false;
bool show_collider_debug = false;

static void debug_input(void* owner, Input_Event* event) {
	const tchar c = event->c;

	switch (c) {
		case CH_KEY_F2:
			show_tile_grid = !show_tile_grid;
			break;
		case CH_KEY_F3: 
			show_transform_origin = !show_transform_origin;
			break;
		case CH_KEY_F4:
			show_collider_debug = !show_collider_debug;
			break;
	}
}

void init_debug() {
	Event_Listener el(nullptr, debug_input, ET_Key_Pressed);
	bind_event_listener(el);
}