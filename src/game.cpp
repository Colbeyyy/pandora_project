#include "game.h"
#include "draw.h"
#include "world.h"
#include "input.h"
#include "console.h"
#include "debug.h"
#include "tile.h"
#include "gui.h"

#include <ch_stl/filesystem.h>
#include <ch_stl/time.h>
#include <ch_stl/pool_allocator.h>
#include <ch_stl/os.h>

ch::Window the_window;
const char* window_title = "newport";
Font font;
const usize temp_arena_size = 1024 * 1024 * 512;

World* loaded_world = nullptr;

static void tick_game(f32 dt) {
	CH_SCOPED_TIMER(tick_game);

	if (loaded_world) loaded_world->tick(dt);
	tick_console(dt);
	tick_debug(dt);
}

#if CH_PLATFORM_WINDOWS
#define MAIN() WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) 
#else
#define MAIN() main() 
#endif

#if CH_PLATFORM_WINDOWS
typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE,
	PROCESS_SYSTEM_DPI_AWARE,
	PROCESS_PER_MONITOR_DPI_AWARE
} PROCESS_DPI_AWARENESS;

typedef HRESULT(*Set_Process_DPI_Awareness)(PROCESS_DPI_AWARENESS value);
#endif

int MAIN() {
	
#if CH_PLATFORM_WINDOWS
	ch::Library shcore_lib;
	if (ch::load_library("shcore.dll", &shcore_lib)) {
		defer(shcore_lib.free());
		Set_Process_DPI_Awareness SetProcessDpiAwareness = shcore_lib.get_function<Set_Process_DPI_Awareness>("SetProcessDpiAwareness");

		if (SetProcessDpiAwareness) {
			SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
		}
	}
#endif


	assert(ch::load_gl());
	{
		const u32 width = 1920;
		const u32 height = (u32)((f32)width * (9.f / 16.f));
		assert(ch::create_gl_window(window_title, width, height, 0, &the_window));
	}
	assert(ch::make_current(the_window));

	the_window.on_sizing = [](const ch::Window& window) {
		draw_game();
	};

	init_am();
	init_draw();
	init_input();
	init_console();
	init_debug();

	loaded_world = ch_new World;
	
	assert(Font::load_from_os("consola.ttf", &font));
	
	ch::context_allocator = ch::make_arena_allocator(temp_arena_size);
	the_window.set_visibility(true);

	f64 last_frame_time = ch::get_time_in_seconds();
	while (!is_exit_requested()) {
		f64 current_frame_time = ch::get_time_in_seconds();
		f32 dt = (f32)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;

		process_input();
		tick_game(dt);
		draw_game();

		ch::scoped_timer_manager.reset();
		refresh_am();
		ch::reset_arena_allocator(&ch::context_allocator);
	}
}