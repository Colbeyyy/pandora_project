#pragma once

#include "texture.h"
#include <ch_stl/math.h>

struct Tile_Grid {
	Texture* tiles_texture;

	static const ch::Vector2 tile_size;
	static ch::Vector2 round_to_grid(ch::Vector2 pos);
	static void debug_draw_grid(ch::Vector2 draw_pos, const ch::Color& color = ch::dark_gray);

};