#pragma once

#include <ch_stl/math.h>

struct Vertical_Layout {
	f32 row_height;
	f32 top_x;
	f32 left_y;
	f32 width;
	f32 at_x;
	f32 at_y;

	Vertical_Layout(f32 _top_x, f32 _left_y, f32 _row_height) : top_x(_top_x), left_y(_left_y), row_height(_row_height), at_x(_top_x), at_y(_left_y) {}

	void row();
};

void gui_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color);
void gui_text(const ch::String& text, f32 x, f32 y, const ch::Color& color);
void gui_text(const char* text, f32 x, f32 y, const ch::Color& color);
bool gui_button(const ch::String& text, f32 x0, f32 y0, f32 x1, f32 y1);
bool gui_button(const char* text, f32 x0, f32 y0, f32 x1, f32 y1);
bool gui_button(struct Texture* tex, f32 x0, f32 y0, f32 x1, f32 y1);

void draw_gui();