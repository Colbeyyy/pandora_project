#include "debug.h"
#include "input.h"
#include "tile.h"
#include "world.h"

bool show_tile_grid = false;

static void debug_input(void* owner, Input_Event* event) {
	const tchar c = event->c;

	switch (c) {
		case CH_KEY_F2:
			show_tile_grid = !show_tile_grid;
			break;
	}
}

void init_debug() {
	Event_Listener el(nullptr, debug_input, ET_Key_Pressed);
	bind_event_listener(el);
}