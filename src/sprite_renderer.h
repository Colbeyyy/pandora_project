#pragma once

#include <ch_stl/array.h>
#include <ch_stl/math.h>
#include "texture.h"

struct Sprite_Renderer {
	struct Render_Command {
		ch::Vector2 position;
		Sprite sprite;
		bool flip_horz;
		Render_Command(ch::Vector2 _position, Sprite _sprite, bool _flip_horz) : position(_position), sprite(_sprite), flip_horz(_flip_horz) {}
	};
	ch::Array<Render_Command> commands;

	Sprite_Renderer() {
		commands.allocator = ch::get_heap_allocator();
	}
	usize push(ch::Vector2 position, Sprite sprite, bool flip_horz);
	void flush();
};

extern Sprite_Renderer sprite_renderer;