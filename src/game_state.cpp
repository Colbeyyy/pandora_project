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

static Game_State g_game_state;

const tchar* window_title = CH_TEXT("pandora_project");

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

	Input_State::bind(&window);

#if CH_PLATFORM_WINDOWS
	wglSwapIntervalEXT(false);
#endif

	Asset_Manager::get().init();

	Imm_Draw::init();

	ch::Allocator temp_allocator = ch::make_arena_allocator(1024 * 1024);
	ch::context_allocator = temp_allocator;

	loaded_world = ch_new World;
	reset_world();

	Font::load_from_os(CH_TEXT("consola.ttf"), &font);
}

void Game_State::loop() {
	f64 last_frame_time = ch::get_time_in_seconds();

    while (!Input_State::get().exit_requested) {
		f64 current_frame_time = ch::get_time_in_seconds();
		dt = (f32)(current_frame_time - last_frame_time);
		last_frame_time = current_frame_time;

		Input_State::process_input();
        tick_game(dt);
        draw_game();

		Asset_Manager::get().refresh();
		ch::reset_arena_allocator(&ch::context_allocator);
    }
}

void Game_State::shut_down() { }

void Game_State::tick_game(f32 dt) {
	if (Input_State::get().was_key_pressed(CH_KEY_R)) {
		reset_world();
	}

	if (Input_State::get().was_key_pressed(CH_KEY_F1)) {
		debug_collision = !debug_collision;
	}

	if (loaded_world) loaded_world->tick(dt);
}

void Game_State::draw_game() {
	Imm_Draw::frame_begin();

	if (loaded_world) loaded_world->draw();

	{
		const ch::Vector2 mouse_pos = loaded_world->current_camera->get_mouse_position_in_world();
		Imm_Draw::draw_quad(ch::Vector2(mouse_pos), 1.f, ch::white);
	}

	Imm_Draw::frame_end();

	ch::swap_buffers(window);
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
	Player* p = loaded_world->spawn_entity<Player>(ch::Vector2(0.f, 0.f));
	player_id = p->id;
}

Game_State& Game_State::get() {
	return g_game_state;
}
