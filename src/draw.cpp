#include "draw.h"
#include "app.h"
#include "console.h"
#include "debug.h"
#include "input.h"
#include "gui.h"

const usize max_verts = 128 * 1024;

u32 draw_calls = 0;
u32 verts_drawn = 0;
u32 verts_culled = 0;

GLuint imm_vao;
GLuint imm_vbo;
Vertex imm_vertices[max_verts];
u32 imm_vertex_count = 0;

ch::Matrix4 projection;
ch::Matrix4 view;

GLuint back_buffer_fbo;
GLuint back_buffer_color;
GLuint back_buffer_depth;
const u32 render_ratio = 4;
u32 back_buffer_width = 4096;
u32 back_buffer_height = 4096;

void init_draw() {
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

	glGenFramebuffers(1, &back_buffer_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, back_buffer_fbo);

	glGenTextures(1, &back_buffer_color);
	glBindTexture(GL_TEXTURE_2D, back_buffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, back_buffer_width, back_buffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, back_buffer_color, 0);

	glGenRenderbuffers(1, &back_buffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, back_buffer_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, back_buffer_width, back_buffer_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, back_buffer_depth);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	wglSwapIntervalEXT(false);
}

static void frame_begin() {
	draw_calls = 0;
	verts_drawn = 0;
	verts_culled = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, back_buffer_fbo);
	const ch::Vector2 viewport_size = the_window.get_viewport_size();
	glClearColor(ch::black);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, back_buffer_width, back_buffer_height);

	render_right_handed();
}

static void frame_end() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(ch::white);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Texture tex;
	tex.id = back_buffer_color;

	const ch::Vector2 viewport_size = the_window.get_viewport_size();
	glViewport(0, 0, viewport_size.ux, viewport_size.uy);

	render_from_pos(0.f, (f32)viewport_size.uy / 2.f);

	{
		Shader* s = find_shader("back_buffer");
		s->bind();
		tex.set_active();
		refresh_transform();

		ch::Vector2 draw_size = get_back_buffer_draw_size();

		const f32 x0 = 0.f - (draw_size.x / 2.f);
		const f32 y0 = 0.f - (draw_size.y / 2.f);
		const f32 x1 = x0 + draw_size.x;
		const f32 y1 = y0 + draw_size.y;

		imm_begin();
		imm_textured_quad(x0, y0, x1, y1, ch::white, tex);
		imm_flush();
	}

}

void draw_game() {
	frame_begin();

	render_from_pos(0.f, back_buffer_width / 2.f);

	draw_quad(0.f, 200.f, ch::white);

	frame_end();

	draw_console();
	draw_gui();

	ch::swap_buffers(the_window);
}

void refresh_transform() {
	const Shader* current_shader = Shader::bound_shader;
	if (!current_shader) return;

	glUniformMatrix4fv(current_shader->view_loc, 1, GL_FALSE, view.elems);
	glUniformMatrix4fv(current_shader->projection_loc, 1, GL_FALSE, projection.elems);
	glUniform1f(current_shader->time_loc, time);
	glUniform2f(current_shader->screen_size_loc, (f32)back_buffer_width, (f32)back_buffer_height);
}

void render_right_handed() {
	const ch::Vector2 viewport_size = the_window.get_viewport_size();

	const f32 width = (f32)viewport_size.ux;
	const f32 height = (f32)viewport_size.uy;
	
	const f32 aspect_ratio = width / height;
	
	const f32 f = 10.f;
	const f32 n = 1.f;
	
	const f32 ortho_size = height / 2.f;

	projection = ch::ortho(ortho_size, aspect_ratio, f, n);
	view = ch::translate(ch::Vector2(-width / 2.f, ortho_size));

	refresh_transform();
}

void render_from_pos(ch::Vector2 pos, f32 ortho_size) {
	const ch::Vector2 viewport_size = the_window.get_viewport_size();
	
	const f32 width = (f32)viewport_size.ux;
	const f32 height = (f32)viewport_size.uy;

	const f32 aspect_ratio = width / height;

	const f32 f = 10.f;
	const f32 n = 1.f;

	projection = ch::ortho(ortho_size, aspect_ratio, f, n);
	view = ch::translate(-pos);

	refresh_transform();
}

void imm_begin() {
	imm_vertex_count = 0;
}

void imm_flush() {
	const Shader* current_shader = Shader::bound_shader;

	assert(current_shader);

	glBindVertexArray(imm_vao);
	glBindBuffer(GL_ARRAY_BUFFER, imm_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(imm_vertices[0]) * imm_vertex_count, imm_vertices, GL_STREAM_DRAW);

	GLuint position_loc = current_shader->position_loc;
	GLuint color_loc = current_shader->color_loc;
	GLuint uv_loc = current_shader->uv_loc;
	GLuint normal_loc = current_shader->normal_loc;
	GLuint z_index_loc = current_shader->z_index_loc;

	glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(position_loc);

	glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(ch::Vector2));
	glEnableVertexAttribArray(color_loc);

	glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(ch::Vector2) + sizeof(ch::Vector4)));
	glEnableVertexAttribArray(uv_loc);

	glVertexAttribPointer(normal_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(ch::Vector2) + sizeof(ch::Vector4) + sizeof(ch::Vector2)));
	glEnableVertexAttribArray(normal_loc);

	glVertexAttribPointer(z_index_loc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(ch::Vector2) + sizeof(ch::Vector4) + sizeof(ch::Vector2) + sizeof(ch::Vector2)));
	glEnableVertexAttribArray(z_index_loc);

	glDrawArrays(GL_TRIANGLES, 0, imm_vertex_count);
	draw_calls += 1;
	verts_drawn += imm_vertex_count;

	glDisableVertexAttribArray(position_loc);
	glDisableVertexAttribArray(color_loc);
	glDisableVertexAttribArray(uv_loc);
	glDisableVertexAttribArray(normal_loc);
	glDisableVertexAttribArray(z_index_loc);

	glBindVertexArray(0);
}

ch::Vector2 get_back_buffer_draw_size() {
	const ch::Vector2 viewport_size = the_window.get_viewport_size();
	f32 width, height;
	const f32 back_buffer_aspect_ratio = (f32)(back_buffer_width) / (f32)(back_buffer_height);
 	const f32 viewport_aspect_ratio = (f32)(viewport_size.ux) / (f32)(viewport_size.uy);

	if (viewport_aspect_ratio >= back_buffer_aspect_ratio) {
		height = (f32)viewport_size.uy;
		width = height * back_buffer_aspect_ratio;
	} else {
		const f32 ratio = (f32)(back_buffer_height) / (f32)(back_buffer_width);
		width = (f32)viewport_size.ux;
		height = width * ratio;
	}

	return ch::Vector2(width, height);
}

void imm_vertex(f32 x, f32 y, const ch::Color& color, ch::Vector2 uv, ch::Vector2 normal, f32 z_index /*= 0.f*/) {
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
	vert->normal = normal;
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

void imm_line(ch::Vector2 start, ch::Vector2 end, f32 thickness, const ch::Color& color, f32 z_index /*= 9.f*/) {

	const ch::Vector2 angle = (end - start).get_normalized();
	const ch::Vector2 perp(angle.y, -angle.x);

	const f32 ht = thickness / 2.f;

	const ch::Vector2 bl = start - perp * ht;
	const ch::Vector2 br = start + perp * ht;
	const ch::Vector2 tl = end - perp * ht;
	const ch::Vector2 tr = end + perp * ht;

	imm_vertex(bl, color, ch::Vector2(-1.f, -1.f), z_index);
	imm_vertex(tl, color, ch::Vector2(-1.f, -1.f), z_index);
	imm_vertex(br, color, ch::Vector2(-1.f, -1.f), z_index);

	imm_vertex(tl, color, ch::Vector2(-1.f, -1.f), z_index);
	imm_vertex(tr, color, ch::Vector2(-1.f, -1.f), z_index);
	imm_vertex(br, color, ch::Vector2(-1.f, -1.f), z_index);
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

void imm_textured_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Texture& texture, f32 z_index) {
	imm_vertex(x0, y0, color, ch::Vector2(0.f, 0.f), ch::Vector2(-1.f, 1.f), z_index);
	imm_vertex(x0, y1, color, ch::Vector2(0.f, 1.f), ch::Vector2(-1.f, 1.f), z_index);
	imm_vertex(x1, y0, color, ch::Vector2(1.f, 0.f), ch::Vector2(1.f, 1.f), z_index);

	imm_vertex(x0, y1, color, ch::Vector2(0.f, 1.f), ch::Vector2(-1.f, 1.f), z_index);
	imm_vertex(x1, y1, color, ch::Vector2(1.f, 1.f), 1.f, 9.f);
	imm_vertex(x1, y0, color, ch::Vector2(1.f, 0.f), ch::Vector2(1.f, 1.f), z_index);
}

void imm_glyph(const Font_Glyph& glyph, f32 x, f32 y, const ch::Color& color, const Font& font, f32 z_index) {
	const f32 x0 = x + glyph.bearing_x;
	const f32 y0 = y - glyph.bearing_y;
	const f32 x1 = x0 + glyph.width;
	const f32 y1 = y0 - glyph.height;

	const ch::Vector2 bottom_right = ch::Vector2(glyph.x1 / (f32)FONT_ATLAS_DIMENSION, glyph.y0 / (f32)FONT_ATLAS_DIMENSION);
	const ch::Vector2 bottom_left = ch::Vector2(glyph.x0 / (f32)FONT_ATLAS_DIMENSION, glyph.y0 / (f32)FONT_ATLAS_DIMENSION);
	const ch::Vector2 top_right = ch::Vector2(glyph.x1 / (f32)FONT_ATLAS_DIMENSION, glyph.y1 / (f32)FONT_ATLAS_DIMENSION);
	const ch::Vector2 top_left = ch::Vector2(glyph.x0 / (f32)FONT_ATLAS_DIMENSION, glyph.y1 / (f32)FONT_ATLAS_DIMENSION);

	imm_vertex(x0, y0, color, bottom_left, 0.f, z_index);
	imm_vertex(x0, y1, color, top_left, 0.f, z_index);
	imm_vertex(x1, y0, color, bottom_right, 0.f, z_index);

	imm_vertex(x0, y1, color, top_left, 0.f, z_index);
	imm_vertex(x1, y1, color, top_right, 0.f, z_index);
	imm_vertex(x1, y0, color, bottom_right, 0.f, z_index);
}

Font_Glyph imm_char(char c, f32 x, f32 y, const ch::Color& color, const Font& font, f32 z_index) {
	const Font_Glyph g = font[c];

	imm_glyph(g, x, y, color, font, z_index);

	return g;
}

ch::Vector2 imm_string(const ch::String& str, f32 x, f32 y, const ch::Color& color, const Font& font, f32 z_index) {
	const f32 font_height = FONT_SIZE;

	const f32 original_x = x;
	const f32 original_y = y;

	f32 largest_x = 0.f;
	f32 largest_y = font_height;

	for (usize i = 0; i < str.count; i++) {
		if (str[i] == ch::eol || str[i] == '\r') {
			y -= font_height;
			largest_y += font_height;
			x = original_x;
			verts_culled += 6;
			continue;
		}

		if (str[i] == '\t') {
			Font_Glyph space_glyph = font[' '];
			x += space_glyph.advance * 4.f;
			verts_culled += 6 * 4;
			continue;
		}

		Font_Glyph& glyph = font[str[i]];

		imm_glyph(glyph, x, y, color, font, z_index);

		x += glyph.advance;

		if (ch::abs(x - original_x) > ch::abs(largest_x)) largest_x = x - original_x;
	}

	return ch::Vector2(largest_x, largest_y);
}

void imm_font_atlas(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Font& font, f32 z_index) {
	imm_textured_quad(x0, y0, x1, y1, color, font.atlas_texture, z_index);
}

void imm_sprite(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Sprite& sprite, bool flip_horz, f32 z_index) {
	assert(sprite.atlas);

	const f32 atlas_w = (f32)sprite.atlas->width;
	const f32 atlas_h = (f32)sprite.atlas->height;

	const f32 atlas_x0 = (f32)(sprite.x * sprite.width);
	const f32 atlas_y0 = (f32)(sprite.y * sprite.height);
	const f32 atlas_x1 = (f32)(atlas_x0 + sprite.width);
	const f32 atlas_y1 = (f32)(atlas_y0 + sprite.height);

	const ch::Vector2 bottom_right = ch::Vector2(atlas_x1 / atlas_w, atlas_y0 / atlas_h);
	const ch::Vector2 bottom_left = ch::Vector2(atlas_x0 / atlas_w, atlas_y0 / atlas_h);
	const ch::Vector2 top_right = ch::Vector2(atlas_x1 / atlas_w, atlas_y1 / atlas_h);
	const ch::Vector2 top_left = ch::Vector2(atlas_x0 / atlas_w, atlas_y1 / atlas_h);

	if (flip_horz) {
		imm_vertex(x0, y0, color, bottom_right, 0.f, z_index);
		imm_vertex(x0, y1, color, top_right, 0.f, z_index);
		imm_vertex(x1, y0, color, bottom_left, 0.f, z_index);

		imm_vertex(x0, y1, color, top_right, 0.f, z_index);
		imm_vertex(x1, y1, color, top_left, 0.f, z_index);
		imm_vertex(x1, y0, color, bottom_left, 0.f, z_index);
	} else {
		imm_vertex(x0, y0, color, bottom_left, 0.f, z_index);
		imm_vertex(x0, y1, color, top_left, 0.f, z_index);
		imm_vertex(x1, y0, color, bottom_right, 0.f, z_index);

		imm_vertex(x0, y1, color, top_left, 0.f, z_index);
		imm_vertex(x1, y1, color, top_right, 0.f, z_index);
		imm_vertex(x1, y0, color, bottom_right, 0.f, z_index);
	}
}
