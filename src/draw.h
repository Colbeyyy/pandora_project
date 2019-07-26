#pragma once

#include <ch_stl/math.h>
#include <ch_stl/opengl.h>

#include "font.h"

void draw_init();
void draw_frame_begin();
void draw_frame_end();

extern ch::Matrix4 view_to_projection;
extern ch::Matrix4 world_to_view;

void refresh_transform();
void render_right_handed();
void render_from_pos(ch::Vector2 pos, f32 ortho_size);

void draw_bind_font(const Font& font);

void imm_begin();
void imm_flush();

void imm_vertex(f32 x, f32 y, const ch::Color& color, ch::Vector2 uv, f32 z_index = 9.f);
CH_FORCEINLINE void imm_vertex(ch::Vector2 xy, const ch::Color& color, ch::Vector2 uv, f32 z_index = 9.f) {
	imm_vertex(xy.x, xy.y, color, uv, z_index);
}
void imm_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index = 9.f);

CH_FORCEINLINE void draw_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index = 9.f) {
	imm_begin();
	imm_quad(x0, y0, x1, y1, color, z_index);
	imm_flush();
}

CH_FORCEINLINE void draw_quad(ch::Vector2 pos, ch::Vector2 size, const ch::Color& color, f32 z_index = 9.f) {
	const f32 x0 = pos.x - (size.x / 2.f);
	const f32 y0 = pos.y - (size.y / 2.f);
	const f32 x1 = x0 + size.x;
	const f32 y1 = y0 + size.y;
	draw_quad(x0, y0, x1, y1, color);
}

void imm_border_quad(f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, const ch::Color& color, f32 z_index = 9.f);

CH_FORCEINLINE void draw_border_quad(f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, const ch::Color& color, f32 z_index = 9.f) {
	imm_begin();
	imm_border_quad(x0, y0, x1, y1, thickness, color, z_index);
	imm_flush();
}

CH_FORCEINLINE void draw_border_quad(ch::Vector2 pos, ch::Vector2 size, f32 thickness, const ch::Color& color, f32 z_index = 9.f) {
	const f32 x0 = pos.x - (size.x / 2.f);
	const f32 y0 = pos.y - (size.y / 2.f);
	const f32 x1 = x0 + size.x;
	const f32 y1 = y0 + size.y;
	draw_border_quad(x0, y0, x1, y1, thickness, color);
}

void imm_line(ch::Vector2 start, ch::Vector2 end, f32 thickness, const ch::Color& color, f32 z_index = 9.f);

CH_FORCEINLINE void draw_line(ch::Vector2 start, ch::Vector2 end, f32 thickness, const ch::Color& color, f32 z_index = 9.f) {
	imm_begin();
	imm_line(start, end, thickness, color, z_index);
	imm_flush();
}

void imm_glyph(const Font_Glyph& glyph, f32 x, f32 y, const ch::Color& color, const Font& font);

CH_FORCEINLINE void draw_glyph(const Font_Glyph& glyph, f32 x, f32 y, const ch::Color& color, const Font& font) {
	imm_begin();
	imm_glyph(glyph, x, y, color, font);
	imm_flush();
}

Font_Glyph imm_char(tchar c, f32 x, f32 y, const ch::Color& color, const Font& font);

CH_FORCEINLINE void draw_char(tchar c, f32 x, f32 y, const ch::Color& color, const Font& font) {
	imm_begin();
	imm_char(c, x, y, color, font);
	imm_flush();
}

ch::Vector2 imm_string(const tchar* str, f32 x, f32 y, const ch::Color& color, const Font& font);

CH_FORCEINLINE void draw_string(const tchar* str, f32 x, f32 y, const ch::Color& color, const Font& font) {
	draw_bind_font(font);
	imm_begin();
	imm_string(str, x, y, color, font);
	imm_flush();
}

// void imm_string();