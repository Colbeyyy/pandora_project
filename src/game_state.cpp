#include "game_state.h"
#include "draw.h"
#include "collision.h"
#include "world.h"
#include "input_state.h"
#include "tile_renderer.h"

#include <ch_stl/window.h>
#include <ch_stl/opengl.h>
#include <ch_stl/filesystem.h>
#include <ch_stl/time.h>
#include <ch_stl/input.h>

#include <stdio.h>

Game_State game_state;

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
        game_state.draw_game();
    };

    window.set_visibility(true);

	input_state.bind(&window);

#if CH_PLATFORM_WINDOWS
	wglSwapIntervalEXT(false);
#endif

	asset_manager.init();

	Imm_Draw::init();

	ch::Allocator temp_allocator = ch::make_arena_allocator(1024 * 1024);
	ch::context_allocator = temp_allocator;

	loaded_world = ch_new World;
	reset_world();

	ch::Array<f32> foo(ch::get_heap_allocator());
	foo.reserve(3);
	foo.reserve(7);

	assert(Font::load_from_os(CH_TEXT("comic.ttf"), &font));
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

	if (input_state.was_key_pressed(CH_KEY_R)) {
		reset_world();
	}

	if (input_state.was_key_pressed(CH_KEY_F1)) {
		debug_collision = !debug_collision;
	}

	if (input_state.was_key_pressed(CH_KEY_F2)) {
		debug_performance = !debug_performance;
	}

	if (loaded_world) loaded_world->tick(dt);
}

void Game_State::draw_game() {
	u32 culled = 0;
	{
		CH_SCOPED_TIMER(draw_game);
		Imm_Draw::frame_begin();
		if (loaded_world) loaded_world->draw();
		culled = tile_renderer.flush();
		Imm_Draw::frame_end();
	}

	if (debug_performance)
	{
		Imm_Draw::render_right_handed();
		const ch::Arena_Allocator_Header* temp_header = ch::context_allocator.get_header<ch::Arena_Allocator_Header>();

		tchar debug_text[4096];
		sprintf(debug_text, CH_TEXT("FPS: %i\nTemp Usage: %llu\nEntity Count: %llu"), (s32)ch::round(1.f / dt), temp_header->current, loaded_world->entities.count);
		f32 x = 10.f;
		f32 y = -20.f;

		Imm_Draw::draw_string(debug_text, x + 2, y - 2, ch::black, font);
		ch::Vector2 size = Imm_Draw::draw_string(debug_text, x, y, ch::white, font);

		y -= size.y + FONT_SIZE;

		for (const ch::Scoped_Timer& it : ch::scoped_timer_manager.entries) {
			const f64 gap = it.get_gap();

			sprintf(debug_text, CH_TEXT("%s : %f"), it.name, gap);
			Imm_Draw::draw_string(debug_text, x + 2, y - 2, ch::black, font);
			size = Imm_Draw::draw_string(debug_text, x, y, ch::white, font);

			y -= FONT_SIZE;
		}

		sprintf(debug_text, CH_TEXT("Tiles Culled: %lu"), culled);
		Imm_Draw::draw_string(debug_text, x + 2, y - 2, ch::black, font);
		size = Imm_Draw::draw_string(debug_text, x, y, ch::white, font);
	}

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
