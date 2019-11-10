#include "font.h"
#include "asset_manager.h"

// @HACK
#include "app.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

bool Font::load_from_os(const char* font_name, Font* out_font) {
	ch::Path font_path = ch::get_os_font_path();
	font_path.append(font_name);
	return load_from_path(font_path, out_font);
}

bool Font::load_from_path(const char* path, Font* out_font) {
	ch::File_Data fd;
	if (!load_asset(path, &fd)) return false;

	Font result;

	stbtt_fontinfo info;
	stbtt_InitFont(&info, fd.data, stbtt_GetFontOffsetForIndex(fd.data, 0));

	Bitmap atlas;
	atlas.width = atlas.height = FONT_ATLAS_DIMENSION;
	atlas.data = ch_new u8[atlas.width * atlas.height];
	atlas.num_components = 1;
	defer(ch_delete atlas.data);

	stbtt_pack_context pc;
	stbtt_packedchar pdata[NUM_CHARACTERS];
	stbtt_pack_range pr;

	stbtt_PackBegin(&pc, atlas.data, atlas.height, atlas.height, 0, 1, NULL);

	const f32 font_size = FONT_SIZE;

	pr.chardata_for_range = pdata;
	pr.array_of_unicode_codepoints = NULL;
	pr.first_unicode_codepoint_in_range = 32;
	pr.num_chars = NUM_CHARACTERS;
	pr.font_size = font_size;

	const u32 h_oversample = 8;
	const u32 v_oversample = 8;

	stbtt_PackSetOversampling(&pc, h_oversample, v_oversample);
	stbtt_PackFontRanges(&pc, fd.data, 0, &pr, 1);
	stbtt_PackEnd(&pc);

	result.atlas_texture = Texture(atlas);

	for (int i = 0; i < NUM_CHARACTERS; i++) {
		Font_Glyph* glyph = &result.glyphs[i];

		glyph->x0 = pdata[i].x0;
		glyph->x1 = pdata[i].x1;
		glyph->y0 = pdata[i].y0;
		glyph->y1 = pdata[i].y1;

		glyph->width = ((f32)pdata[i].x1 - pdata[i].x0) / (float)h_oversample;
		glyph->height = ((f32)pdata[i].y1 - pdata[i].y0) / (float)v_oversample;
		glyph->bearing_x = pdata[i].xoff;
		glyph->bearing_y = pdata[i].yoff;
		glyph->advance = pdata[i].xadvance;
	}

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);

	const f32 font_scale = stbtt_ScaleForPixelHeight(&info, font_size);
	result.ascent = (f32)ascent * font_scale;
	result.descent = (f32)descent * font_scale;
	result.line_gap = (f32)line_gap * font_scale;

	*out_font = result;
	return true;
}

ch::Vector2 get_text_size(const ch::String& text) {
	const f32 font_height = FONT_SIZE;

	const f32 original_x = 0.f;
	const f32 original_y = 0.f;

	f32 x = original_x;
	f32 y = original_y;

	f32 largest_x = 0.f;
	f32 largest_y = 0.f;

	for (usize i = 0; i < text.count; i++) {
		if (text[i] == ch::eol) {
			y -= font_height;
			x = original_x;
			continue;
		}

		if (text[i] == '\t') {
			Font_Glyph space_glyph = font[' '];
			x += space_glyph.advance * 4.f;
			continue;
		}

		Font_Glyph& glyph = font[text[i]];
		x += glyph.advance;

		if (ch::abs(x - original_x) > ch::abs(largest_x)) largest_x = x - original_x;
		if (ch::abs(y - original_y) > ch::abs(largest_y)) largest_y = x - original_y;
	}

	return ch::Vector2(largest_x, largest_y);
}
