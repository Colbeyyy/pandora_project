#include "tile_renderer.h"
#include "draw.h"
#include "collision.h"
#include "entity.h"
#include "game_state.h"
#include "world.h"

#include <ch_stl/time.h>

#include <stdio.h>

Tile_Renderer tile_renderer;

Tile_Renderer::Tile_Renderer() {
	commands.allocator = ch::get_heap_allocator();
}

void Tile_Renderer::push_tile(ch::Vector2 position, ch::Vector2 size) {
	Render_Command rc;
	rc.position = position;
	rc.size = size;
	commands.push(rc);
}

u32 Tile_Renderer::flush() {
	CH_SCOPED_TIMER(tile_renderer_flush);
	Shader* s = asset_manager.find_shader(CH_TEXT("image"));
	s->bind();
	Texture* t = asset_manager.find_texture(CH_TEXT("rock_tile"));
	t->set_active();
	Camera* c = game_state.loaded_world->current_camera;
	const ch::Vector2 draw_size = get_back_buffer_draw_size() / 4.f;
	AABB render_bounds(c->position.xy, draw_size);
	
	u32 culled = 0;

	imm_begin();
	for (const Render_Command& it : commands) {
		AABB bounds(it.position, it.size);
		if (!bounds.intersects(render_bounds)) {
			culled += 1;
			continue;
		}

		const f32 x0 = it.position.x - (it.size.x / 2.f);
		const f32 y0 = it.position.y - (it.size.y / 2.f);
		const f32 x1 = x0 + it.size.x;
		const f32 y1 = y0 + it.size.y;
		imm_textured_quad(x0, y0, x1, y1, ch::white, *t);
	}
	imm_flush();
	commands.count = 0;

	return culled;
}
