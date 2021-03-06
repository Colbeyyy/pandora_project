#pragma once

#include "texture.h"

#define NUM_CHARACTERS 95
#define FONT_ATLAS_DIMENSION 2048
#define FONT_SIZE 22.f

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
		if (index >= NUM_CHARACTERS + 32) {
			return glyphs['?' - 32];
		}
		return glyphs[index - 32];
	}

	static bool load_from_os(const char* font_name, Font* out_font);
	static bool load_from_path(const char* path, Font* out_font);
};

ch::Vector2 get_text_size(const ch::String& text);
CH_FORCEINLINE ch::Vector2 get_text_size(const char* text) {
	ch::String s;
	s.data = (char*)text;
	s.count = ch::strlen(text);
	return get_text_size(s);
}