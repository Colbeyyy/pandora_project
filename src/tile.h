#pragma once

#include <ch_stl/math.h>
#include <ch_stl/hash_table.h>

struct Tile_Grid {
	static const ch::Vector2 tile_size;

	static ch::Vector2 round_to_grid(ch::Vector2 pos);
};

struct Tile_Renderer {
	struct Render_Command {
		ch::Vector2 position;
		s32 index;
	};

	ch::Array<Render_Command> commands;

	Tile_Renderer() {
		commands.allocator = ch::get_heap_allocator();
	}

	void push(ch::Vector2 position, s32 index);
	void flush();

};

extern Tile_Renderer tile_renderer;