#pragma once

#include <ch_stl/math.h>

struct Tile_Grid {
	static const ch::Vector2 tile_size;

	static ch::Vector2 round_to_grid(ch::Vector2 pos);
};