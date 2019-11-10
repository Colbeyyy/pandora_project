#include "app.h"
#include "draw.h"
#include "input.h"
#include "console.h"
#include "debug.h"
#include "gui.h"

#include <ch_stl/filesystem.h>
#include <ch_stl/time.h>
#include <ch_stl/pool_allocator.h>
#include <ch_stl/os.h>

ch::Window the_window;
const char* window_title = "Shader Playground";
Font font;
const usize temp_arena_size = 1024 * 1024 * 512;
float time = 0.f;

bool paused = false;

static void do_gui(f32 dt) {

	const ch::Vector2 viewport_size = the_window.get_viewport_size();
	const f32 bar_height = 40.f;

	{
		const ch::Color bar_color = 0x433b4dff;

		const f32 x0 = 0.f;
		const f32 y0 = -(f32)viewport_size.uy + bar_height;
		const f32 x1 = x0 + (f32)viewport_size.ux;
		const f32 y1 = y0 - bar_height;

		gui_quad(x0, y0, x1, y1, bar_color);
	}

	const f32 padding_x = 15.f;
	f32 current_x = 0.f;
	const f32 button_size = 32.f;

	const float button_offset_y = (bar_height - button_size) / 2.f;

	{
		const f32 x0 = current_x + (padding_x / 2.f);
		const f32 y0 = -(f32)viewport_size.uy + button_offset_y;
		const f32 x1 = x0 + button_size;
		const f32 y1 = y0 + button_size;

		current_x += button_size + padding_x / 2.f;

		Texture* t = find_texture("reset");

		if (gui_button(t, x0, y0, x1, y1)) {
			time = 0.f;
		}
	}

	{
		const f32 x0 = current_x + (padding_x / 2.f);
		const f32 y0 = -(f32)viewport_size.uy + button_offset_y;
		const f32 x1 = x0 + button_size;
		const f32 y1 = y0 + button_size;

		current_x += button_size + padding_x / 2.f;

		Texture* t = paused ? find_texture("play") : find_texture("pause");

		if (gui_button(t, x0, y0, x1, y1)) {
			paused = !paused;
		}
	}

	current_x += 40.f;

	{
		const f32 x0 = current_x + (padding_x / 2.f);
		const f32 y0 = -(f32)viewport_size.uy + bar_height - 1.f;

		current_x += button_size + padding_x / 2.f;

		char buffer[100];
		ch::sprintf(buffer, "%.1fs  %.1fFPS", time, 1.f / dt);

		gui_text(buffer, x0, y0, ch::white);
	}
	
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
		do_gui(0.f);
		draw_app();
	};

	init_am();
	init_draw();
	init_input();
	init_console();
	init_debug();
	
	assert(Font::load_from_os("consola.ttf", &font));
	
	ch::context_allocator = ch::make_arena_allocator(temp_arena_size);
	the_window.set_visibility(true);

	f64 last_frame_time = ch::get_time_in_seconds();
	while (!is_exit_requested()) {
		f64 current_frame_time = ch::get_time_in_seconds();
		f32 dt = (f32)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;

		process_input();
		tick_console(dt);
		tick_debug(dt);

		if (!paused) {
			time += dt;
		}

		do_gui(dt);
		draw_app();

		ch::scoped_timer_manager.reset();
		refresh_am();
		ch::reset_arena_allocator(&ch::context_allocator);
	}
}