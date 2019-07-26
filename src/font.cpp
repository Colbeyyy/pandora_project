#include "font.h"

#include <ch_stl/os.h>
#include <ch_stl/memory.h>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

// @NOTE(CHall): Move this elsewhere
struct Bitmap {
	s32 width, height;
	u8* data;
};

bool Font::load_from_os(const tchar* font_name, Font* out_font) {
	ch::String old_path = ch::get_current_path();
	defer({
		ch::set_current_path(old_path);
		old_path.destroy();
	});
	ch::String font_path = ch::get_os_font_path();
	defer(font_path.destroy());
	assert(ch::set_current_path(font_path));

	return load_from_path(font_name, out_font);
}

bool Font::load_from_path(const tchar* path, Font* out_font) {
	FILE* f = fopen(path, "rb");
	if (!f) {
		return false;
	}
	fseek(f, 0, SEEK_END);
	const usize size = ftell(f);
	fseek(f, 0, SEEK_SET);

	u8* buffer = ch_new u8[size];
	defer(ch_delete buffer);
	fread(buffer, size, 1, f);
	fclose(f);
	buffer[size] = 0;

	Font result;

	stbtt_fontinfo info;
	stbtt_InitFont(&info, buffer, stbtt_GetFontOffsetForIndex(buffer, 0));

	Bitmap atlas;
	atlas.width = atlas.height = FONT_ATLAS_DIMENSION;
	atlas.data = (u8*)ch::malloc(atlas.width * atlas.height);

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
	stbtt_PackFontRanges(&pc, buffer, 0, &pr, 1);
	stbtt_PackEnd(&pc);

	glGenTextures(1, &result.texture_id);
	glBindTexture(GL_TEXTURE_2D, result.texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas.width, atlas.height, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data);

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

	glBindTexture(GL_TEXTURE_2D, 0);

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);

	const f32 font_scale = stbtt_ScaleForPixelHeight(&info, font_size);
	result.ascent = (f32)ascent * font_scale;
	result.descent = (f32)descent * font_scale;
	result.line_gap = (f32)line_gap * font_scale;

	*out_font = result;
	return true;
}
