#pragma once

#include <ch_stl/math.h>
#include <ch_stl/opengl.h>


struct Vertex {
	ch::Vector2 position;
	ch::Color color;
	ch::Vector2 uv;
	f32 z_index;
};

struct Shader {
	GLuint program_id;

	GLint view_to_projection_loc;
	GLint world_to_view_loc;

	GLint position_loc;
	GLint color_loc;
	GLint uv_loc;
	GLint z_index_loc;

	GLuint texture_loc;

	static Shader* bound_shader;
	static bool load_from_source(const GLchar* source, Shader* out_shader);

	void bind();
	void unbind();
	CH_FORCEINLINE bool is_bound() const {
		return Shader::bound_shader == this;
	}
};

struct Bitmap {
	s32 width, height;
	u8* data;
	s32 num_components;

	CH_FORCEINLINE explicit operator bool() const { return data && width > 0 && height > 0;}
};

enum Texture_Type {
	BT_Red = 1,
	BT_RGB = 3,
	BT_RGBA = 4,
};

struct Texture {
	GLuint id;

	ch::File_Data image_memory;

	Texture() = default;
	Texture(const Bitmap& bm);

	static Texture* bound_texture;
	static Texture* active_texture;
	static bool load_from_path(const tchar* path, Texture_Type load_type, Texture* out_texture);

	void bind();
	void unbind();

	void set_active();

	CH_FORCEINLINE bool is_bound() const {
		return Texture::bound_texture == this;
	}

	CH_FORCEINLINE bool is_active() const {
		return Texture::active_texture == this;
	}
};

#define NUM_CHARACTERS 95
#define FONT_ATLAS_DIMENSION 2048
#define FONT_SIZE 20.f;

struct Font_Glyph {
	f32 width, height;
	f32 bearing_x, bearing_y;
	f32 advance;

	u32 x0, y0, x1, y1;
};

struct Font {
	f32 ascent;
	f32 descent;
	f32 line_gap;

	Texture atlas_texture;
	Font_Glyph glyphs[NUM_CHARACTERS];

	CH_FORCEINLINE void bind() {
		atlas_texture.set_active();
	}

	CH_FORCEINLINE bool is_bound() const { return atlas_texture.is_active(); }

	CH_FORCEINLINE Font_Glyph operator[](usize index) const {
		assert(index < NUM_CHARACTERS + 32);
		return glyphs[index - 32];
	}

	static bool load_from_os(const tchar* font_name, Font* out_font);
	static bool load_from_path(const tchar* path, Font* out_font);
};

namespace Imm_Draw {
	void init();

	void frame_begin();
	void frame_end();

	void refresh_transform();
	void render_right_handed();
	void render_from_pos(ch::Vector2 pos, f32 ortho_size);

	extern ch::Matrix4 view_to_projection;
	extern ch::Matrix4 world_to_view;

	extern Shader solid_shape_shader;
	extern Shader font_shader;
	extern Shader image_shader;

	extern Texture test;

	void imm_begin();
	void imm_flush();

	void imm_vertex(f32 x, f32 y, const ch::Color& color, ch::Vector2 uv, f32 z_index = 9.f);
	CH_FORCEINLINE void imm_vertex(ch::Vector2 xy, const ch::Color& color, ch::Vector2 uv, f32 z_index = 9.f) {
		imm_vertex(xy.x, xy.y, color, uv, z_index);
	}
	void imm_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index = 9.f);

	CH_FORCEINLINE void draw_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index = 9.f) {
		Imm_Draw::solid_shape_shader.bind();
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
		Imm_Draw::solid_shape_shader.bind();
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
		Imm_Draw::solid_shape_shader.bind();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_line(start, end, thickness, color, z_index);
		Imm_Draw::imm_flush();
	}

	void imm_textured_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Texture& texture);

	CH_FORCEINLINE void draw_textured_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, Texture& texture) {
		Imm_Draw::image_shader.bind();
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
		Imm_Draw::font_shader.bind();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_glyph(glyph, x, y, color, font);
		Imm_Draw::imm_flush();
	}

	Font_Glyph imm_char(tchar c, f32 x, f32 y, const ch::Color& color, const Font& font);

	CH_FORCEINLINE void draw_char(tchar c, f32 x, f32 y, const ch::Color& color, Font& font) {
		Imm_Draw::font_shader.bind();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_char(c, x, y, color, font);
		Imm_Draw::imm_flush();
	}

	ch::Vector2 imm_string(const tchar* str, f32 x, f32 y, const ch::Color& color, const Font& font);

	CH_FORCEINLINE void draw_string(const tchar* str, f32 x, f32 y, const ch::Color& color, Font& font) {
		Imm_Draw::font_shader.bind();
		font.bind();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_string(str, x, y, color, font);
		Imm_Draw::imm_flush();
	}

	void imm_font_atlas(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Font& font);

	CH_FORCEINLINE void draw_font_atlas(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, Font& font) {
		Imm_Draw::font_shader.bind();
		font.bind();
		Imm_Draw::imm_begin();
		Imm_Draw::imm_font_atlas(x0, y0, x1, y1, color, font);
		Imm_Draw::imm_flush();
	}
}