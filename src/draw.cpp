#include "draw.h"
#include "game_state.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Shader* Shader::bound_shader = nullptr;

bool Shader::load_from_source(const GLchar* source, Shader* out_shader) {
	Shader result;
	GLuint program_id = glCreateProgram();

	GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* shader_header = "#version 330 core\n#extension GL_ARB_separate_shader_objects: enable\n";

	const GLchar* vert_shader[3] = { shader_header, "#define VERTEX 1", source };
	const GLchar* frag_shader[3] = { shader_header, "#define FRAGMENT 1", source };

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

	result.view_to_projection_loc = glGetUniformLocation(program_id, "view_to_projection");
	result.world_to_view_loc = glGetUniformLocation(program_id, "world_to_view");
	result.texture_loc = glGetUniformLocation(program_id, "ftex");
	result.position_loc = 0;
	result.color_loc = 1;
	result.uv_loc = 2;
	result.z_index_loc = 3;

	glUniform1i(result.texture_loc, 0);

	*out_shader = result;

	return true;
}

void Shader::bind() {
	glUseProgram(program_id);
	Shader::bound_shader = this;
	Imm_Draw::refresh_transform();
}

void Shader::unbind() {
	glUseProgram(0);
	Shader::bound_shader = nullptr;
}

const usize max_verts = 3 * 1024;

u32 draw_calls = 0;
u32 verts_drawn = 0;
u32 verts_culled = 0;

GLuint imm_vao;
GLuint imm_vbo;
Vertex imm_vertices[max_verts];
u32 imm_vertex_count = 0;

ch::Matrix4 Imm_Draw::view_to_projection;
ch::Matrix4 Imm_Draw::world_to_view;

Shader Imm_Draw::solid_shape_shader;
Shader Imm_Draw::font_shader;
Shader Imm_Draw::image_shader;
Texture Imm_Draw::test;

const GLchar* solid_shape_source= R"foo(
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

const GLchar* font_source = R"foo(
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
	vec4 result = texture(ftex, out_uv);
	result.w = 1.0;
	frag_color = vec4(out_color.xyz, texture(ftex, out_uv).r);
}
#endif
)foo";

const GLchar* image_source = R"foo(
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
	vec4 result = texture(ftex, out_uv);
	frag_color = result * out_color;
}
#endif
)foo";

Texture::Texture(const Bitmap& bm) {
	glGenTextures(1, &id);

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

bool Texture::load_from_path(const tchar* path, Texture_Type load_type, Texture* out_texture) {
	if (!g_game_state.asset_manager.load_asset(path, &out_texture->image_memory)) return false;

	const s32 desired_components = (s32)load_type;

	stbi_set_flip_vertically_on_load(true);

	Bitmap bm;
	bm.data = stbi_load_from_memory(out_texture->image_memory.data, (int)out_texture->image_memory.size, &bm.width, &bm.height, &bm.num_components, desired_components);
	defer(stbi_image_free(bm.data));
	if (!bm) return false;

	*out_texture = Texture(bm);

	return true;
}

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

bool Font::load_from_os(const tchar* font_name, Font* out_font) {
	ch::String old_path = ch::get_current_path();
	defer(ch::set_current_path(old_path));
	ch::String font_path = ch::get_os_font_path();

	if (!ch::set_current_path(font_path)) {
		return false;
	}

	return load_from_path(font_name, out_font);
}

bool Font::load_from_path(const tchar* path, Font* out_font) {

	ch::File_Data fd;
	if (!g_game_state.asset_manager.load_asset(path, &fd)) return false;

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

void Imm_Draw::init() {
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

	Shader::load_from_source(solid_shape_source, &solid_shape_shader);
	Shader::load_from_source(font_source, &font_shader);
	Shader::load_from_source(image_source, &image_shader);

	Asset_Manager::set_to_res_path();

	ch::String current_path = ch::get_current_path();

	Texture::load_from_path(CH_TEXT("..\\res\\tex\\rock_tile.png"), BT_RGBA, &test);

	solid_shape_shader.bind();

	render_right_handed();
}

void Imm_Draw::frame_begin() {
	draw_calls = 0;
	verts_drawn = 0;
	verts_culled = 0;

	const ch::Vector2 viewport_size = g_game_state.window.get_viewport_size();
	glViewport(0, 0, viewport_size.ux, viewport_size.uy);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(ch::black);

}

void Imm_Draw::frame_end() {
	ch::swap_buffers(g_game_state.window);
}

void Imm_Draw::refresh_transform() {
	const Shader* current_shader = Shader::bound_shader;
	assert(current_shader);

	glUniformMatrix4fv(current_shader->world_to_view_loc, 1, GL_FALSE, world_to_view.elems);
	glUniformMatrix4fv(current_shader->view_to_projection_loc, 1, GL_FALSE, view_to_projection.elems);
}

void Imm_Draw::render_right_handed() {
	const ch::Vector2 viewport_size = g_game_state.window.get_viewport_size();

	const f32 width = (f32)viewport_size.ux;
	const f32 height = (f32)viewport_size.uy;
	
	const f32 aspect_ratio = width / height;
	
	const f32 f = 10.f;
	const f32 n = 1.f;
	
	const f32 ortho_size = height / 2.f;

	view_to_projection = ch::ortho(ortho_size, aspect_ratio, f, n);
	world_to_view = ch::translate(ch::Vector2(-width / 2.f, ortho_size));

	Imm_Draw::refresh_transform();
}

void Imm_Draw::render_from_pos(ch::Vector2 pos, f32 ortho_size) {
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

void Imm_Draw::imm_begin() {
	imm_vertex_count = 0;
}

void Imm_Draw::imm_flush() {
	const Shader* current_shader = Shader::bound_shader;

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

void Imm_Draw::imm_vertex(f32 x, f32 y, const ch::Color& color, ch::Vector2 uv, f32 z_index /*= 0.f*/) {
	auto get_next_vertex_ptr = []() -> Vertex* {
		return (Vertex*)&imm_vertices + imm_vertex_count;
	};

	if (imm_vertex_count >= max_verts) {
		Imm_Draw::imm_flush();
		Imm_Draw::imm_begin();
	}

	Vertex* vert = get_next_vertex_ptr();

	vert->position.x = x;
	vert->position.y = y;
	vert->color = color;
	vert->uv = uv;
	vert->z_index = z_index;

	imm_vertex_count += 1;
}

void Imm_Draw::imm_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index /*= 0.f*/) {
	Imm_Draw::imm_vertex(x0, y0, color, ch::Vector2(-1.f, -1.f), z_index);
	Imm_Draw::imm_vertex(x0, y1, color, ch::Vector2(-1.f, -1.f), z_index);
	Imm_Draw::imm_vertex(x1, y0, color, ch::Vector2(-1.f, -1.f), z_index);

	Imm_Draw::imm_vertex(x0, y1, color, ch::Vector2(-1.f, -1.f), z_index);
	Imm_Draw::imm_vertex(x1, y1, color, ch::Vector2(-1.f, -1.f), z_index);
	Imm_Draw::imm_vertex(x1, y0, color, ch::Vector2(-1.f, -1.f), z_index);
}

void Imm_Draw::imm_line(ch::Vector2 start, ch::Vector2 end, f32 thickness, const ch::Color& color, f32 z_index /*= 9.f*/) {

	const ch::Vector2 angle = (end - start).get_normalized();
	const ch::Vector2 perp(angle.y, -angle.x);

	const f32 ht = thickness / 2.f;

	const ch::Vector2 bl = start - perp * ht;
	const ch::Vector2 br = start + perp * ht;
	const ch::Vector2 tl = end - perp * ht;
	const ch::Vector2 tr = end + perp * ht;

	Imm_Draw::imm_vertex(bl, color, ch::Vector2(-1.f, -1.f), z_index);
	Imm_Draw::imm_vertex(tl, color, ch::Vector2(-1.f, -1.f), z_index);
	Imm_Draw::imm_vertex(br, color, ch::Vector2(-1.f, -1.f), z_index);

	Imm_Draw::imm_vertex(tl, color, ch::Vector2(-1.f, -1.f), z_index);
	Imm_Draw::imm_vertex(tr, color, ch::Vector2(-1.f, -1.f), z_index);
	Imm_Draw::imm_vertex(br, color, ch::Vector2(-1.f, -1.f), z_index);
}

void Imm_Draw::imm_border_quad(f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, const ch::Color& color, f32 z_index /*= 9.f*/)
{
	{
		const f32 _x0 = x0;
		const f32 _y0 = y0;
		const f32 _x1 = _x0 + thickness;
		const f32 _y1 = y1;
		Imm_Draw::imm_quad(_x0, _y0, _x1, _y1, color, z_index);
	}

	{
		const f32 _x0 = x1 - thickness;
		const f32 _y0 = y0;
		const f32 _x1 = _x0 + thickness;
		const f32 _y1 = y1;
		Imm_Draw::imm_quad(_x0, _y0, _x1, _y1, color, z_index);
	}

	{
		const f32 _x0 = x0;
		const f32 _y0 = y0;
		const f32 _x1 = x1;
		const f32 _y1 = _y0 + thickness;
		Imm_Draw::imm_quad(_x0, _y0, _x1, _y1, color, z_index);
	}

	{
		const f32 _x0 = x0;
		const f32 _y0 = y1 - thickness;
		const f32 _x1 = x1;
		const f32 _y1 = _y0 + thickness;
		Imm_Draw::imm_quad(_x0, _y0, _x1, _y1, color, z_index);
	}
}

void Imm_Draw::imm_textured_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Texture& texture) {
	Imm_Draw::imm_vertex(x0, y0, color, ch::Vector2(0.f, 0.f), 9.f);
	Imm_Draw::imm_vertex(x0, y1, color, ch::Vector2(0.f, 1.f), 9.f);
	Imm_Draw::imm_vertex(x1, y0, color, ch::Vector2(1.f, 0.f), 9.f);

	Imm_Draw::imm_vertex(x0, y1, color, ch::Vector2(0.f, 1.f), 9.f);
	Imm_Draw::imm_vertex(x1, y1, color, ch::Vector2(1.f, 1.f), 9.f);
	Imm_Draw::imm_vertex(x1, y0, color, ch::Vector2(1.f, 0.f), 9.f);
}

void Imm_Draw::imm_glyph(const Font_Glyph& glyph, f32 x, f32 y, const ch::Color& color, const Font& font) {
	const f32 x0 = x + glyph.bearing_x;
	const f32 y0 = y - glyph.bearing_y;
	const f32 x1 = x0 + glyph.width;
	const f32 y1 = y0 - glyph.height;

	const ch::Vector2 bottom_right = ch::Vector2(glyph.x1 / (f32)FONT_ATLAS_DIMENSION, glyph.y0 / (f32)FONT_ATLAS_DIMENSION);
	const ch::Vector2 bottom_left = ch::Vector2(glyph.x0 / (f32)FONT_ATLAS_DIMENSION, glyph.y0 / (f32)FONT_ATLAS_DIMENSION);
	const ch::Vector2 top_right = ch::Vector2(glyph.x1 / (f32)FONT_ATLAS_DIMENSION, glyph.y1 / (f32)FONT_ATLAS_DIMENSION);
	const ch::Vector2 top_left = ch::Vector2(glyph.x0 / (f32)FONT_ATLAS_DIMENSION, glyph.y1 / (f32)FONT_ATLAS_DIMENSION);

	Imm_Draw::imm_vertex(x0, y0, color, bottom_left);
	Imm_Draw::imm_vertex(x0, y1, color, top_left);
	Imm_Draw::imm_vertex(x1, y0, color, bottom_right);

	Imm_Draw::imm_vertex(x0, y1, color, top_left);
	Imm_Draw::imm_vertex(x1, y1, color, top_right);
	Imm_Draw::imm_vertex(x1, y0, color, bottom_right);
}

Font_Glyph Imm_Draw::imm_char(tchar c, f32 x, f32 y, const ch::Color& color, const Font& font) {
	const Font_Glyph g = font[c];

	imm_glyph(g, x, y, color, font);

	return g;
}

ch::Vector2 Imm_Draw::imm_string(const tchar* str, f32 x, f32 y, const ch::Color& color, const Font& font) {
	const f32 font_height = FONT_SIZE;

	const f32 original_x = x;
	const f32 original_y = y;

	f32 largest_x = 0.f;
	f32 largest_y = 0.f;

	for (usize i = 0; i < ch::strlen(str); i++) {
		if (str[i] == ch::eol) {
			y -= font_height;
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

		Imm_Draw::imm_glyph(glyph, x, y, color, font);

		x += glyph.advance;

		if (x - original_x > largest_x) largest_x = x - original_x;
		if (y - original_y > largest_y) largest_y = x - original_y;
	}

	return ch::Vector2(largest_x, largest_y);
}

void Imm_Draw::imm_font_atlas(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, const Font& font) {
	imm_textured_quad(x0, y0, x1, y1, color, font.atlas_texture);
}