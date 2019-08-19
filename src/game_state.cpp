#include "game_state.h"
#include "draw.h"
#include "collision.h"
#include "world.h"
#include "input_state.h"

#include <ch_stl/window.h>
#include <ch_stl/opengl.h>
#include <ch_stl/filesystem.h>
#include <ch_stl/time.h>
#include <ch_stl/input.h>
#include <ch_stl/hash_table.h>

#include <stdio.h>

Game_State game_state;

const tchar* window_title = CH_TEXT("pandora_project");

void Game_State::init() {
    assert(ch::load_gl());
    {
        const u32 width = 1920;
        const u32 height = (u32)((f32)width * (9.f / 16.f));
        assert(ch::create_gl_window(window_title, width, height, 0, &window));
    }
    assert(ch::make_current(window));

    window.on_sizing = [](const ch::Window& window) {
        game_state.draw_game();
    };

    window.set_visibility(true);

	input_state.bind(&window);

#if CH_PLATFORM_WINDOWS
	wglSwapIntervalEXT(true);
#endif

	asset_manager.init();

	draw_init();

	ch::Allocator temp_allocator = ch::make_arena_allocator(1024 * 1024);
	ch::context_allocator = temp_allocator;

	loaded_world = ch_new World;

	{
		assert(Font::load_from_path(CH_TEXT("fonts/FORCED SQUARE.ttf"), &font));
	}
}

void Game_State::loop() {
	f64 last_frame_time = ch::get_time_in_seconds();

    while (!input_state.exit_requested) {
		f64 current_frame_time = ch::get_time_in_seconds();
		dt = (f32)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;

		process_input();
        tick_game(dt);
        draw_game();

		ch::scoped_timer_manager.reset();
		asset_manager.refresh();
		ch::reset_arena_allocator(&ch::context_allocator);

		Sleep(30);
    }
}

void Game_State::shut_down() { }

void Game_State::process_input() {
	input_state.reset();
	ch::poll_events();
	input_state.process_input();
}

void Game_State::tick_game(f32 dt) {
	CH_SCOPED_TIMER(tick_game);

	if (loaded_world) loaded_world->tick(dt);
}

void Game_State::draw_game() {
	u32 culled = 0;
	{
		CH_SCOPED_TIMER(draw_game);
		frame_begin();
		if (loaded_world) loaded_world->draw();
		frame_end();
	}

	ch::swap_buffers(window);
}