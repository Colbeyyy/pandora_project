#include "shader.h"

GLuint Shader::bound_shader;

bool Shader::load_from_source(const GLchar* source, Shader* out_shader) {
	Shader result;
	GLuint program_id = glCreateProgram();

	GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* shader_header = "#version 330 core\n#extension GL_ARB_separate_shader_objects: enable\n";

	const GLchar* vert_shader[3] = { shader_header, "#define VERTEX 1\n", source };
	const GLchar* frag_shader[3] = { shader_header, "#define FRAGMENT 1\n", source };

	glShaderSource(vertex_id, 3, vert_shader, 0);
	glShaderSource(frag_id, 3, frag_shader, 0);

	glCompileShader(vertex_id);
	glCompileShader(frag_id);

	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, frag_id);

	glLinkProgram(program_id);

	glValidateProgram(program_id);

	GLint is_linked = false;
	glGetProgramiv(program_id, GL_LINK_STATUS, &is_linked);
	if (!is_linked) {
		GLsizei ignored;
		char vert_errors[4096];
		char frag_errors[4096];
		char program_errors[4096];

		glGetShaderInfoLog(vertex_id, sizeof(vert_errors), &ignored, vert_errors);
		glGetShaderInfoLog(frag_id, sizeof(frag_errors), &ignored, frag_errors);
		glGetProgramInfoLog(program_id, sizeof(program_errors), &ignored, program_errors);
		return false;
	}

	glDeleteShader(vertex_id);
	glDeleteShader(frag_id);

	result.program_id = program_id;

	result.projection_loc = glGetUniformLocation(program_id, "projection");
	result.view_loc = glGetUniformLocation(program_id, "view");
	result.texture_loc = glGetUniformLocation(program_id, "ftex");
	result.position_loc = 0;
	result.color_loc = 1;
	result.uv_loc = 2;
	result.normal_loc = 3;
	result.z_index_loc = 4;

	glUniform1i(result.texture_loc, 0);

	*out_shader = result;

	return true;
}

void Shader::free() {
	if (is_bound()) unbind();

	if (*this) {
		glDeleteProgram(program_id);
		program_id = 0;
	}
}

void Shader::bind() {
	glUseProgram(program_id);
	Shader::bound_shader = program_id;
}

void Shader::unbind() {
	glUseProgram(0);
	Shader::bound_shader = 0;
}
