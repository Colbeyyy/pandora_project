#pragma once

#include <ch_stl/opengl.h>

#define NUM_CHARACTERS 95

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

	GLuint texture_id;
	Font_Glyph glyphs[NUM_CHARACTERS];

	CH_FORCEINLINE Font_Glyph operator[](usize index) const {
		assert(index < NUM_CHARACTERS);
		return glyphs[index - 32];
	}

	static bool load_from_os(const tchar* font_name, Font* out_font);
};