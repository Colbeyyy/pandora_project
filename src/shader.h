#pragma once

#include <ch_stl/opengl.h>

struct Shader {
	GLuint program_id;

	GLint projection_loc;
	GLint view_loc;

	GLint position_loc;
	GLint color_loc;
	GLint uv_loc;
	GLint normal_loc;
	GLint z_index_loc;

	GLuint texture_loc;

	static GLuint bound_shader;
	static bool load_from_source(const GLchar* source, Shader* out_shader);

	CH_FORCEINLINE operator bool() const { return program_id != 0;}
	void free();

	void bind();
	void unbind();
	CH_FORCEINLINE bool is_bound() const {
		return Shader::bound_shader == program_id;
	}
};