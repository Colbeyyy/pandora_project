#include "game_state.h"
#include "draw.h"
#include "entity.h"

Game_State g_game_state;

#include <ch_stl/window.h>
#include <ch_stl/opengl.h>
#include <ch_stl/filesystem.h>
#include <ch_stl/time.h>

const tchar* window_title = CH_TEXT("pandora_project");

f32 x = 0.f;
const f32 speed = 5.f;

void Game_State::init() {
    assert(ch::load_gl());
    {
        const u32 width = 1920;
        const u32 height = (u32)((f32)width * (9.f / 16.f));
        assert(ch::create_gl_window(window_title, width, height, 0, &window));
    }
    assert(ch::make_current(window));

    window.on_exit_requested = [](const ch::Window& window) {
        g_game_state.exit_requested = true;
    };
    window.on_sizing = [](const ch::Window& window) {
        g_game_state.draw_game();
    };

    window.set_visibility(true);

#if CH_PLATFORM_WINDOWS
	wglSwapIntervalEXT(!BUILD_DEBUG);
#endif
	draw_init();

	loaded_world = ch_new World;

	Camera* cam = loaded_world->spawn_entity<Camera>(0.f);
	cam->set_to_current();
	loaded_world->spawn_entity<Block>(0.f);
}

void Game_State::loop() {
	f64 last_frame_time = ch::get_ms_time();
    while (!exit_requested) {
		f64 current_frame_time = ch::get_ms_time();
		const f32 dt = (f32)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;
        process_inputs();
        tick_game(dt);
        draw_game();
    }
}

void Game_State::shut_down() {

}

void Game_State::process_inputs() {
    ch::poll_events();
}

void Game_State::tick_game(f32 dt) {
	if (loaded_world) loaded_world->tick(dt);
}

void Game_State::draw_game() {
	draw_frame_begin();

	if (loaded_world) loaded_world->draw();

	draw_frame_end();
}