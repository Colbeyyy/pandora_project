#include "game_state.h"
#include "draw.h"

Game_State g_game_state;

#include <ch_stl/ch_window.h>
#include <ch_stl/ch_opengl.h>

const tchar* window_title = CH_TEXT("pandora_project");

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
    window.on_resize = [](const ch::Window& window) {
        // glViewport(0, 0, window.width, window.height);
    };

    window.set_visibility(true);

	draw_init();
}

void Game_State::loop() {
    while (!exit_requested) {
        process_inputs();
        tick_game(0.f);
        draw_game();
    }
}

void Game_State::shut_down() {

}

void Game_State::process_inputs() {
    ch::poll_events();
}

void Game_State::tick_game(f32 dt) {
}

void Game_State::draw_game() {
	draw_frame_begin();

	render_right_handed();

	imm_begin();

	imm_quad(0.f, 0.f, 100.f, 100.f, ch::Vector4(1.f, 0.f, 1.f, 1.f), 9.f);

	imm_flush();

	draw_frame_end();
}