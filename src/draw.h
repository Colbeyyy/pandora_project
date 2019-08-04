#pragma once

#include <ch_stl/math.h>
#include <ch_stl/opengl.h>

#include "asset.h"
#include "shader.h"
#include "texture.h"
#include "font.h"

struct Vertex {
	ch::Vector2 position;
	ch::Color color;
	ch::Vector2 uv;
	ch::Vector2 normal;
	f32 z_index;
};

namespace Imm_Draw {
	void init();

	void frame_begin();
	void frame_end();

	void refresh_transform();
	void render_right_handed();
	void render_from_pos(ch::Vector2 pos, f32 ortho_size);

	extern ch::Matrix4 projection;
	extern ch::Matrix4 view;

	extern GLuint back_buffer_fbo;
	extern GLuint back_buffer_color;
	extern GLuint back_buffer_depth;
	extern u32 back_buffer_width;
	extern u32 back_buffer_height;

	void imm_begin();
	void imm_flush();

	ch::Vector2 get_back_buffer_draw_size();

	void imm_vertex(f32 x, f32 y, const ch::Color& color, ch::Vector2 uv, ch::Vector2 normal = 0.f, f32 z_index = 9.f);
	CH_FORCEINLINE void imm_vertex(ch::Vector2 xy, const ch::Color& color, ch::Vector2 uv, ch::Vector2 normal = 0.f, f32 z_index = 9.f) {
		imm_vertex(xy.x, xy.y, color, uv, normal, z_index);
	}
	void imm_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index = 9.f);

	CH_FORCEINLINE void draw_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index = 9.f) {
		Shader* shader = asset_manager.find_shader(CH_TEXT("solid_shape"));
		assert(shader);
		shader->bind();
		refresh_transform();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_quad(x0, y0, x1, y1, color, z_index);
		Imm_Draw::imm_flush();
	}

	CH_FORCEINLINE void draw_quad(ch::Vector2 pos, ch::Vector2 size, const ch::Color& color, f32 z_index = 9.f) {
		const f32 x0 = pos.x - (size.x / 2.f);
		const f32 y0 = pos.y - (size.y / 2.f);
		const f32 x1 = x0 + size.x;
		const f32 y1 = y0 + size.y;
		Imm_Draw::draw_quad(x0, y0, x1, y1, color);
	}

	void imm_border_quad(f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, const ch::Color& color, f32 z_index = 9.f);

	CH_FORCEINLINE void draw_border_quad(f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, const ch::Color& color, f32 z_index = 9.f) {
		Shader* shader = asset_manager.find_shader(CH_TEXT("solid_shape"));
		assert(shader);
		shader->bind();
		refresh_transform();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_border_quad(x0, y0, x1, y1, thickness, color, z_index);
		Imm_Draw::imm_flush();
	}

	CH_FORCEINLINE void draw_border_quad(ch::Vector2 pos, ch::Vector2 size, f32 thickness, const ch::Color& color, f32 z_index = 9.f) {
		const f32 x0 = pos.x - (size.x / 2.f);
		const f32 y0 = pos.y - (size.y / 2.f);
		const f32 x1 = x0 + size.x;
		const f32 y1 = y0 + size.y;
		Imm_Draw::draw_border_quad(x0, y0, x1, y1, thickness, color);
	}

	void imm_line(ch::Vector2 start, ch::Vector2 end, f32 thickness, const ch::Color& color, f32 z_index = 9.f);

	CH_FORCEINLINE void draw_line(ch::Vector2 start, ch::Vector2 end, f32 thickness, const ch::Color& color, f32 z_index = 9.f) {
		Shader* shader = asset_manager.find_shader(CH_TEXT("solid_shape"));
		assert(shader);
		shader->bind();
		refresh_transform();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_line(start, end, thickness, color, z_index);
		Imm_Draw::imm_flush();
	}

	void imm_textured_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Texture& texture);

	CH_FORCEINLINE void draw_textured_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, Texture& texture) {
		Shader* shader = asset_manager.find_shader(CH_TEXT("image"));
		assert(shader);
		shader->bind();
		refresh_transform();
		texture.set_active();

		Imm_Draw::imm_begin();
		Imm_Draw::imm_textured_quad(x0, y0, x1, y1, color, texture);
		Imm_Draw::imm_flush();
	}

	CH_FORCEINLINE void draw_textured_quad(ch::Vector2 pos, ch::Vector2 size, const ch::Color& color, Texture& texture) {
		const f32 x0 = pos.x - (size.x / 2.f);
		const f32 y0 = pos.y - (size.y / 2.f);
		const f32 x1 = x0 + size.x;
		const f32 y1 = y0 + size.y;
		Imm_Draw::draw_textured_quad(x0, y0, x1, y1, color, texture);
	}

	void imm_glyph(const Font_Glyph& glyph, f32 x, f32 y, const ch::Color& color, const Font& font);

	CH_FORCEINLINE void draw_glyph(const Font_Glyph& glyph, f32 x, f32 y, const ch::Color& color, Font& font) {
		Shader* shader = asset_manager.find_shader(CH_TEXT("font"));
		assert(shader);
		shader->bind();
		refresh_transform();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_glyph(glyph, x, y, color, font);
		Imm_Draw::imm_flush();
	}

	Font_Glyph imm_char(tchar c, f32 x, f32 y, const ch::Color& color, const Font& font);

	CH_FORCEINLINE void draw_char(tchar c, f32 x, f32 y, const ch::Color& color, Font& font) {
		Shader* shader = asset_manager.find_shader(CH_TEXT("font"));
		assert(shader);
		shader->bind();
		refresh_transform();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_char(c, x, y, color, font);
		Imm_Draw::imm_flush();
	}

	ch::Vector2 imm_string(const tchar* str, f32 x, f32 y, const ch::Color& color, const Font& font);

	CH_FORCEINLINE ch::Vector2 draw_string(const tchar* str, f32 x, f32 y, const ch::Color& color, Font& font) {
		Shader* shader = asset_manager.find_shader(CH_TEXT("font"));
		assert(shader);
		shader->bind();
		refresh_transform();
		font.bind();
		Imm_Draw::imm_begin();
		const ch::Vector2 result = Imm_Draw::imm_string(str, x, y, color, font);
		Imm_Draw::imm_flush();

		return result;
	}

	void imm_font_atlas(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Font& font);

	CH_FORCEINLINE void draw_font_atlas(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, Font& font) {
		Shader* shader = asset_manager.find_shader(CH_TEXT("font"));
		assert(shader);
		shader->bind();
		refresh_transform();
		font.bind();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_font_atlas(x0, y0, x1, y1, color, font);
		Imm_Draw::imm_flush();
	}
}