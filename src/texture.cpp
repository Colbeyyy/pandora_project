#include "texture.h"

Texture::Texture(const Bitmap& bm) {
	glGenTextures(1, &id);

	width = bm.width;
	height = bm.height;

	bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLint format = 0;

	switch (bm.num_components) {
	case 1:
		format = GL_RED;
		break;
	case 2:
		format = GL_RG;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		assert(false);
		break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, format, bm.width, bm.height, 0, format, GL_UNSIGNED_BYTE, bm.data);
}

Texture* Texture::bound_texture = nullptr;
Texture* Texture::active_texture = nullptr;

void Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, id);
	Texture::bound_texture = this;
}

void Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
	Texture::bound_texture = nullptr;
}

void Texture::set_active() {
	bind();
	glActiveTexture(GL_TEXTURE0);
	active_texture = this;
}

void Texture::free() {
	unbind();

	glDeleteTextures(1, &id);
}
