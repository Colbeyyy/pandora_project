#include "tile.h"

#include "asset.h"
#include "draw.h"

#include "world.h"

#include "collision.h"

const ch::Vector2 Tile_Grid::tile_size = 16.f;

ch::Vector2 Tile_Grid::round_to_grid(ch::Vector2 pos) {
	pos /= tile_size;
	pos = ch::round(pos);
	pos *= tile_size;

	return pos;
}

Tile_Renderer tile_renderer;

void Tile_Renderer::push(ch::Vector2 position, s32 index) {
	Render_Command rc;
	rc.position = position;
	rc.index = index;
	commands.push(rc);
}

void Tile_Renderer::flush() {
	Shader* s = asset_manager.find_shader(CH_TEXT("image"));
	s->bind();

	s->bind();
	Texture* t = asset_manager.find_texture(CH_TEXT("test_tilesheet"));
	t->set_active();

	refresh_transform();

	Camera* c = get_world()->current_camera;
	AABB render_bounds(c->position, c->size);

	imm_begin();
	for (const Render_Command& it : commands) {
		AABB bounds(it.position, Tile_Grid::tile_size);
		if (!bounds.intersects(render_bounds)) {
			continue;
		}

		const f32 x0 = it.position.x - (Tile_Grid::tile_size.x / 2.f);
		const f32 y0 = it.position.y - (Tile_Grid::tile_size.y / 2.f);
		const f32 x1 = x0 + Tile_Grid::tile_size.x;
		const f32 y1 = y0 + Tile_Grid::tile_size.y;

		Sprite sprite(t, Tile_Grid::tile_size.x, Tile_Grid::tile_size.y, 0, 0);

		imm_sprite(x0, y0, x1, y1, ch::white, sprite);
	}
	imm_flush();
}
