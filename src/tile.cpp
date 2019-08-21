#include "tile.h"

const ch::Vector2 Tile_Grid::tile_size = 16.f;

ch::Vector2 Tile_Grid::round_to_grid(ch::Vector2 pos) {
	pos /= tile_size;
	pos = ch::round(pos);
	pos *= tile_size;

	return pos;
}