#pragma once

#include <ch_stl/opengl.h>

struct Bitmap {
	s32 width, height;
	u8* data;
	s32 num_components;

	CH_FORCEINLINE explicit operator bool() const { return data && width > 0 && height > 0; }
};

enum Texture_Type {
	BT_Red = 1,
	BT_RGB = 3,
	BT_RGBA = 4,
};

struct Texture {
	GLuint id;

	u32 width;
	u32 height;

	Texture() = default;
	Texture(const Bitmap& bm);

	static Texture* bound_texture;
	static Texture* active_texture;

	void bind();
	void unbind();

	void set_active();

	void free();

	CH_FORCEINLINE bool is_bound() const {
		return Texture::bound_texture == this;
	}

	CH_FORCEINLINE bool is_active() const {
		return Texture::active_texture == this;
	}
};

struct Sprite {
	Texture* atlas;

	u32 width;
	u32 height;

	u32 x;
	u32 y;

	Sprite() = default;
	Sprite(Texture* _atlas, u32 _w, u32 _h, u32 _x, u32 _y) : atlas(_atlas), width(_w), height(_h), x(_x), y(_y) {}
};