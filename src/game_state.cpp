#include "game_state.h"
#include "draw.h"
#include "collision.h"
#include "world.h"

#include <ch_stl/window.h>
#include <ch_stl/opengl.h>
#include <ch_stl/filesystem.h>
#include <ch_stl/time.h>

Game_State g_game_state;
Input_State g_input_state;

const tchar* window_title = CH_TEXT("pandora_project");

void Input_State::bind(ch::Window* window) {
	window->on_exit_requested = [](const ch::Window& window) {
		g_input_state.exit_requested = true;
	};

	window->on_key_pressed = [](const ch::Window& window, u8 key) {
		if (!g_input_state.keys_down[key]) {
			g_input_state.keys_pressed[key] = true;
		}
		g_input_state.keys_down[key] = true;
	};

	window->on_key_released = [](const ch::Window& window, u8 key) {
		g_input_state.keys_down[key] = false;
	};

	window->on_mouse_button_down = [](const ch::Window& window, u8 mouse_button) {
		if (!g_input_state.mb_down[mouse_button]) {
			g_input_state.mb_pressed[mouse_button] = true;
		}
		g_input_state.mb_down[mouse_button] = true;
	};

	window->on_mouse_button_up = [](const ch::Window& window, u8 mouse_button) {
		g_input_state.mb_down[mouse_button] = false;
	};
}

void Input_State::reset() {
	ch::mem_set(keys_pressed, sizeof(keys_pressed), 0);
	ch::mem_set(mb_pressed, sizeof(mb_pressed), 0);
}

void Game_State::init() {
    assert(ch::load_gl());
    {
        const u32 width = 1280;
        const u32 height = (u32)((f32)width * (9.f / 16.f));
        assert(ch::create_gl_window(window_title, width, height, 0, &window));
    }
    assert(ch::make_current(window));

    window.on_sizing = [](const ch::Window& window) {
        g_game_state.draw_game();
    };

    window.set_visibility(true);

	g_input_state.bind(&window);

#if CH_PLATFORM_WINDOWS
	wglSwapIntervalEXT(!BUILD_DEBUG);
#endif
	draw_init();

	loaded_world = ch_new World;
	reset_world();
}

void Game_State::loop() {
	f64 last_frame_time = ch::get_ms_time();
    while (!g_input_state.exit_requested) {
		f64 current_frame_time = ch::get_ms_time();
		dt = (f32)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;
        process_inputs();
        tick_game(dt);
        draw_game();
    }
}

void Game_State::shut_down() {

}

void Game_State::process_inputs() {
	g_input_state.reset();

    ch::poll_events();

	ch::Vector2 mouse_pos;
	if (!window.get_mouse_position(&mouse_pos)) {
		mouse_pos.ux = 0;
		mouse_pos.uy = 0;
	}
	g_input_state.current_mouse_position = ch::Vector2((f32)mouse_pos.ux, (f32)mouse_pos.uy);
}

void Game_State::tick_game(f32 dt) {

	if (g_input_state.was_key_pressed('R')) {
		reset_world();
	}

	if (loaded_world) loaded_world->tick(dt);
}

void Game_State::draw_game() {
	draw_frame_begin();

	if (loaded_world) loaded_world->draw();

	{


		draw_quad(loaded_world->current_camera->get_mouse_position_in_world(), 10.f, ch::white);
	}

	draw_frame_end();
}

void Game_State::reset_world() {
	loaded_world->destroy_all();

	Camera* cam = loaded_world->spawn_entity<Camera>(0.f);
	cam->set_to_current();
	{
		Block* b = loaded_world->spawn_entity<Block>(ch::Vector2(0.f, -200.f));
		b->size = ch::Vector2(10000.f, 100.f);
	}
	{
		Block* b = loaded_world->spawn_entity<Block>(ch::Vector2(200.f, -100.f));
		b->size = 100.f;
	}
	{
		Block* b = loaded_world->spawn_entity<Block>(ch::Vector2(300.f, -50.f));
		b->size = ch::Vector2(100.f, 200.f);
	}
	{
		Block* b = loaded_world->spawn_entity<Block>(ch::Vector2(500.f, 50.f));
		b->size = 100.f;
	}
	Player* p = loaded_world->spawn_entity<Player>(ch::Vector2(0.f, 100.f));
	player_id = p->id;
}
