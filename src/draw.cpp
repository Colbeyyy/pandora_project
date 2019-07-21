#include "draw.h"
#include "game_state.h"

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

	static Shader* current_shader;
};

Shader* Shader::current_shader = nullptr;

const usize max_verts = 3 * 1024;

u32 draw_calls = 0;
u32 verts_drawn = 0;
u32 verts_culled = 0;

GLuint imm_vao;
GLuint imm_vbo;
Vertex imm_vertices[max_verts];
u32 imm_vertex_count = 0;

ch::Matrix4 view_to_projection;
ch::Matrix4 world_to_view;

Shader solid_shape;

const GLchar* shader = R"foo(
#extension GL_ARB_separate_shader_objects: enable

#ifdef VERTEX
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in float z_index;
uniform mat4 view_to_projection;
uniform mat4 world_to_view;
out vec4 out_color;
out vec2 out_uv;
void main() {
    gl_Position =  view_to_projection * world_to_view * vec4(position, -z_index, 1.0);
	out_color = color;
	out_uv = uv;
}
#endif

#ifdef FRAGMENT
out vec4 frag_color;
in vec4 out_color;
in vec2 out_uv;
uniform sampler2D ftex;
void main() {
	if (out_uv.x < 0 || out_uv.y < 0) {
		frag_color = out_color;
	} else {
		vec4 result = texture(ftex, out_uv);
		result.w = 1.0;
		frag_color = vec4(out_color.xyz, texture(ftex, out_uv).r);
	}
}
#endif
)foo";

static Shader load_shader(const GLchar* shader_source) {
	Shader result;
	GLuint program_id = glCreateProgram();

	GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* shader_header = "#version 330 core\n";

	const GLchar* vert_shader[3] = { shader_header, "#define VERTEX 1", shader_source };
	const GLchar* frag_shader[3] = { shader_header, "#define FRAGMENT 1", shader_source };

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
		assert(!"Shader validation failed");
	}

	glDeleteShader(vertex_id);
	glDeleteShader(frag_id);

	result.program_id = program_id;

	result.view_to_projection_loc = glGetUniformLocation(program_id, "view_to_projection");
	result.world_to_view_loc = glGetUniformLocation(program_id, "world_to_view");
	result.texture_loc = glGetUniformLocation(program_id, "ftex");
	result.position_loc = 0;
	result.color_loc = 1;
	result.uv_loc = 2;
	result.z_index_loc = 3;

	return result;
}

void draw_init() {
	assert(ch::is_gl_loaded());

	glGenVertexArrays(1, &imm_vao);
	glBindVertexArray(imm_vao);

	glGenBuffers(1, &imm_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, imm_vbo);

	glBindVertexArray(0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);

	solid_shape = load_shader(shader);
	glUseProgram(solid_shape.program_id);
	Shader::current_shader = &solid_shape;

	render_right_handed();
}

void draw_frame_begin() {
	draw_calls = 0;
	verts_drawn = 0;
	verts_culled = 0;

	const ch::Vector2 viewport_size = g_game_state.window.get_viewport_size();
	glViewport(0, 0, viewport_size.ux, viewport_size.uy);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(ch::black);

}

void draw_frame_end() {
	ch::swap_buffers(g_game_state.window);
}

void refresh_transform() {
	const Shader* current_shader = Shader::current_shader;
	assert(current_shader);

	glUniformMatrix4fv(current_shader->world_to_view_loc, 1, GL_FALSE, world_to_view.elems);
	glUniformMatrix4fv(current_shader->view_to_projection_loc, 1, GL_FALSE, view_to_projection.elems);
}

void render_right_handed() {
	const ch::Vector2 viewport_size = g_game_state.window.get_viewport_size();

	const f32 width = (f32)viewport_size.ux;
	const f32 height = (f32)viewport_size.uy;
	
	const f32 aspect_ratio = width / height;
	
	const f32 f = 10.f;
	const f32 n = 1.f;
	
	const f32 ortho_size = height / 2.f;

	view_to_projection = ch::ortho(ortho_size, aspect_ratio, f, n);
	world_to_view = ch::translate(ch::Vector2(-width / 2.f, ortho_size));

	refresh_transform();
}

void render_from_pos(ch::Vector2 pos, f32 ortho_size) {
	const ch::Vector2 viewport_size = g_game_state.window.get_viewport_size();
	
	const f32 width = (f32)viewport_size.ux;
	const f32 height = (f32)viewport_size.uy;

	const f32 aspect_ratio = width / height;

	const f32 f = 10.f;
	const f32 n = 1.f;

	view_to_projection = ch::ortho(ortho_size, aspect_ratio, f, n);
	world_to_view = ch::translate(-pos);

	refresh_transform();
}

void imm_begin() {
	imm_vertex_count = 0;
}

void imm_flush() {
	const Shader* current_shader = Shader::current_shader;

	assert(current_shader);

	glBindVertexArray(imm_vao);
	glBindBuffer(GL_ARRAY_BUFFER, imm_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(imm_vertices[0]) * imm_vertex_count, imm_vertices, GL_STREAM_DRAW);

	GLuint position_loc = current_shader->position_loc;
	GLuint color_loc = current_shader->color_loc;
	GLuint uv_loc = current_shader->uv_loc;
	GLuint z_index_loc = current_shader->z_index_loc;

	glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(position_loc);

	glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(ch::Vector2));
	glEnableVertexAttribArray(color_loc);

	glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(ch::Vector2) + sizeof(ch::Vector4)));
	glEnableVertexAttribArray(uv_loc);

	glVertexAttribPointer(z_index_loc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(ch::Vector2) + sizeof(ch::Vector4) + sizeof(ch::Vector2)));
	glEnableVertexAttribArray(z_index_loc);

	glDrawArrays(GL_TRIANGLES, 0, imm_vertex_count);
	draw_calls += 1;
	verts_drawn += imm_vertex_count;

	glDisableVertexAttribArray(position_loc);
	glDisableVertexAttribArray(color_loc);
	glDisableVertexAttribArray(uv_loc);
	glDisableVertexAttribArray(z_index_loc);

	glBindVertexArray(0);
}

void imm_vertex(f32 x, f32 y, const ch::Color& color, ch::Vector2 uv, f32 z_index /*= 0.f*/) {
	auto get_next_vertex_ptr = []() -> Vertex* {
		return (Vertex*)&imm_vertices + imm_vertex_count;
	};

	if (imm_vertex_count >= max_verts) {
		imm_flush();
		imm_begin();
	}

	Vertex* vert = get_next_vertex_ptr();

	vert->position.x = x;
	vert->position.y = y;
	vert->color = color;
	vert->uv = uv;
	vert->z_index = z_index;

	imm_vertex_count += 1;
}

void imm_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index /*= 0.f*/) {
	imm_vertex(x0, y0, color, ch::Vector2(-1.f, -1.f), z_index);
	imm_vertex(x0, y1, color, ch::Vector2(-1.f, -1.f), z_index);
	imm_vertex(x1, y0, color, ch::Vector2(-1.f, -1.f), z_index);

	imm_vertex(x0, y1, color, ch::Vector2(-1.f, -1.f), z_index);
	imm_vertex(x1, y1, color, ch::Vector2(-1.f, -1.f), z_index);
	imm_vertex(x1, y0, color, ch::Vector2(-1.f, -1.f), z_index);
}

void imm_border_quad(f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, const ch::Color& color, f32 z_index /*= 9.f*/)
{
	{
		const f32 _x0 = x0;
		const f32 _y0 = y0;
		const f32 _x1 = _x0 + thickness;
		const f32 _y1 = y1;
		imm_quad(_x0, _y0, _x1, _y1, color, z_index);
	}

	{
		const f32 _x0 = x1 - thickness;
		const f32 _y0 = y0;
		const f32 _x1 = _x0 + thickness;
		const f32 _y1 = y1;
		imm_quad(_x0, _y0, _x1, _y1, color, z_index);
	}

	{
		const f32 _x0 = x0;
		const f32 _y0 = y0;
		const f32 _x1 = x1;
		const f32 _y1 = _y0 + thickness;
		imm_quad(_x0, _y0, _x1, _y1, color, z_index);
	}

	{
		const f32 _x0 = x0;
		const f32 _y0 = y1 - thickness;
		const f32 _x1 = x1;
		const f32 _y1 = _y0 + thickness;
		imm_quad(_x0, _y0, _x1, _y1, color, z_index);
	}
}
