#pragma once

#include "texture.h"
#include <ch_stl/math.h>

struct Tile {
	u32 x;
	u32 y;
};

const u32 tile_chunk_size = 16;
struct Tile_Chunk {
	Tile tiles[tile_chunk_size * tile_chunk_size];
};

struct Tile_Grid {
	Texture* tiles_texture;

	static const f32 tile_size;
	static ch::Vector2 round_to_grid(ch::Vector2 pos);
	static void debug_draw_grid(ch::Vector2 draw_pos, const ch::Color& color = ch::dark_gray);

};