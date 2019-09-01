#include "tile.h"
#include "draw.h"

const ch::Vector2 Tile_Grid::tile_size = 16.f;
const ch::Vector2 Tile_Chunk::chunk_size = 16.f;

ch::Vector2 Tile_Grid::round_to_grid(ch::Vector2 pos) {
	pos /= tile_size;
	pos = ch::round(pos);
	pos *= tile_size;

	return pos;
}

void Tile_Grid::debug_draw_grid(ch::Vector2 draw_pos, const ch::Color& color /*= ch::dark_gray*/) {
	Shader* s = find_shader("solid_shape");
	s->bind();
	refresh_transform();
	imm_begin();
	const f32 half_width = (f32)back_buffer_width / 2.f;
	const f32 half_height = (f32)back_buffer_height / 2.f;
	for (f32 x = draw_pos.x - half_width; x < draw_pos.x + half_width; x += tile_size.x) {
		const f32 rounded_x = round_to_grid(ch::Vector2(x, 0.f)).x;
		const ch::Vector2 start(rounded_x, draw_pos.y + half_height);
		const ch::Vector2 end(rounded_x, draw_pos.y - half_height);
		imm_line(start, end, 1.f, color);
	}

	for (f32 y = draw_pos.y - half_width; y < draw_pos.y + half_width; y += tile_size.y) {
		const f32 rounded_y = round_to_grid(ch::Vector2(0.f, y)).y;
		const ch::Vector2 start(draw_pos.x - half_width, rounded_y);
		const ch::Vector2 end(draw_pos.x + half_width, rounded_y);
		imm_line(start, end, 1.f, color);
	}
	imm_flush();
}