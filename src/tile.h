#pragma once

#include <ch_stl/math.h>
#include <ch_stl/hash_table.h>

struct Tile {

};

struct Tile_Grid {
	static const ch::Vector2 tile_size;

	ch::Hash_Table<ch::Vector2, Tile> tiles;

	CH_FORCEINLINE Tile* find_tile(s32 x, s32 y) {
		ch::Vector2 loc;
		loc.ix = x;
		loc.iy = y;

		return tiles.find(loc);
	}

	static ch::Vector2 round_to_grid(ch::Vector2 pos);
};