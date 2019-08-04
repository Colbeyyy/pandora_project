#pragma once

#include <ch_stl/math.h>

struct Render_Command {
	ch::Vector2 position;
	ch::Vector2 size;
};

struct Tile_Renderer {
	ch::Array<Render_Command> commands;

	Tile_Renderer();

	void push_tile(ch::Vector2 position, ch::Vector2 size);
	void flush();
};

extern Tile_Renderer tile_renderer;