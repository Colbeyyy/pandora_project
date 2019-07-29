#include "game_state.h"
#include "draw.h"
#include "collision.h"
#include "world.h"

#include <ch_stl/window.h>
#include <ch_stl/opengl.h>
#include <ch_stl/filesystem.h>
#include <ch_stl/time.h>
#include <ch_stl/input.h>

#include <stdio.h>

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
	wglSwapIntervalEXT(false);
#endif
	asset_manager = Asset_Manager(1024 * 1024);

	Imm_Draw::init();

	ch::Allocator temp_allocator = ch::make_arena_allocator(1024 * 32);
	ch::context_allocator = temp_allocator;

	loaded_world = ch_new World;
	reset_world();

	Font::load_from_os(CH_TEXT("consola.ttf"), &font);
}

void Game_State::loop() {
	f64 last_frame_time = ch::get_ms_time();
	f32 dt_counter = 0.f;
	u32 fps_count = 0;
    while (!g_input_state.exit_requested) {
		f64 current_frame_time = ch::get_ms_time();
		dt = (f32)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;


		dt_counter += dt;
		if (dt_counter > 1.f) {
			fps = fps_count;
			dt_counter = 0.f;
			fps_count = 0;
		}

        process_inputs();
        tick_game(dt);
        draw_game();

		ch::reset_arena_allocator(&ch::context_allocator);

		fps_count += 1;
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

	if (g_input_state.was_key_pressed(CH_KEY_ESCAPE)) {
		g_input_state.exit_requested = true;
	}

	if (loaded_world) loaded_world->tick(dt);
}

void Game_State::draw_game() {
	Imm_Draw::frame_begin();

	if (loaded_world) loaded_world->draw();

	{
		Imm_Draw::test.set_active();
		Imm_Draw::image_shader.bind();
		Imm_Draw::imm_begin();

		const ch::Vector2 size = 16.f;

		for (f32 x = -16.f * 50.f; x < 16.f * 50.f; x += 16.f) {
			for (f32 y = -16.f * 3.f; y > -16.f * 7.f; y -= 16.f) {
				const f32 x0 = x - (size.x / 2.f);
				const f32 y0 = y - (size.y / 2.f);
				const f32 x1 = x0 + size.x;
				const f32 y1 = y0 + size.y;
				Imm_Draw::imm_textured_quad(x0, y0, x1, y1, ch::white, Imm_Draw::test);
			}
		}

		Imm_Draw::imm_flush();
	}

	{
		Imm_Draw::draw_quad(loaded_world->current_camera->get_mouse_position_in_world(), 1.f, ch::white);
	}

	{
		Imm_Draw::render_right_handed();
		tchar buffer[512];
#if CH_UNICODE
		swprintf(buffer, CH_TEXT("FPS: %i\nEntity Count: %i"), fps, loaded_world->entities.count);
#else
		sprintf(buffer, "FPS: %i\nEntity Count: %i\nAssets Loaded: %ikb\n", fps, loaded_world->entities.count, asset_manager.get_current_size() / 1024);
#endif
		Imm_Draw::draw_string(buffer, 12.f, -22.f, ch::black, font);
		Imm_Draw::draw_string(buffer, 10.f, -20.f, ch::white, font);
		// Imm_Draw::draw_string(ch::get_current_path(), 10.f, -100.f, ch::white, font);
	}

	Imm_Draw::frame_end();
}

void Game_State::reset_world() {
	loaded_world->destroy_all();

	Camera* cam = loaded_world->spawn_entity<Camera>(0.f);
	cam->set_to_current();
	{
		f32 current_x = -(16.f * 50);
		for (usize i = 0; i < 100; i++) {
			Block* b = loaded_world->spawn_entity<Block>(ch::Vector2(current_x, -32.f));

			const f32 size = 16.f;

			b->size = size;
			current_x += size;
		}
	}
	{
		Block* b = loaded_world->spawn_entity<Block>(ch::Vector2(32.f, -16.f));
		b->size = 16.f;
	}
	{
		Block* b = loaded_world->spawn_entity<Block>(ch::Vector2(48.f, -16.f));
		b->size = 16.f;
	}
	{
		Block* b = loaded_world->spawn_entity<Block>(ch::Vector2(48.f, 0.f));
		b->size = 16.f;
	}
	Player* p = loaded_world->spawn_entity<Player>(ch::Vector2(0.f, 100.f));
	player_id = p->id;
}
