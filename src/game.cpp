#include "game.h"
#include "draw.h"
#include "world.h"
#include "input.h"
#include "console.h"

#include <ch_stl/filesystem.h>
#include <ch_stl/time.h>
#include <ch_stl/pool_allocator.h>

ch::Window the_window;
const tchar* window_title = CH_TEXT("pandora_project");
Font font;
const usize temp_arena_size = 1024 * 1024 * 512;

World* loaded_world = nullptr;

static void tick_game(f32 dt) {
	CH_SCOPED_TIMER(tick_game);

	tick_console(dt);
	if (loaded_world) loaded_world->tick(dt);
	
}


#if CH_PLATFORM_WINDOWS && !BUILD_DEBUG
#define MAIN() WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) 
#else
#define MAIN() main() 
#endif

int MAIN() {
	assert(ch::load_gl());
	{
		const u32 width = 1280;
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

	loaded_world = ch_new World;

	{
		Entity* cam = get_world()->spawn_entity();
		cam->add_component<Transform_Component>();
		Camera_Component* cam_c = cam->add_component<Camera_Component>();

		loaded_world->cam_id = cam->id;

		Entity* sp = get_world()->spawn_entity();
		sp->add_component<Transform_Component>();
		Sprite_Component* s = sp->add_component<Sprite_Component>();

		Texture* t = find_texture(CH_TEXT("test_tilesheet"));
		Sprite sprite(t, 16, 16, 0, 0);
		s->sprite = sprite;
	}
	assert(Font::load_from_os(CH_TEXT("consola.ttf"), &font));
	
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