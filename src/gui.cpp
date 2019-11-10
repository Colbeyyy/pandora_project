#include "gui.h"
#include "draw.h"
#include "app.h"
#include "input.h"

enum Render_Type {
	RT_Quad,
	RT_Border_Quad,
	RT_Text,
};

struct Render_Command {
	Render_Type type;
	ch::Color color;
	f32 z_index = 9.f;
	union {
		struct {
			f32 x0, y0, x1, y1;
			f32 thickness;
		};
		struct {
			f32 x, y;
			ch::String text;
		};
	};

	Render_Command() {}
};

static ch::Array<Render_Command> commands(ch::get_heap_allocator());
static const ch::Color button_color = 0x523957FF;
static const ch::Color hovered_button_color = 0x392338FF;

static void push_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Color& color, f32 z_index = 9.f) {
	Render_Command rc;
	rc.type = RT_Quad;
	rc.color = color;
	rc.z_index = z_index;
	rc.x0 = x0;
	rc.y0 = y0;
	rc.x1 = x1;
	rc.y1 = y1;
	commands.push(rc);
}

static void push_quad(ch::Vector2 position, ch::Vector2 size, const ch::Color& color, f32 z_index = 9.f) {
	Render_Command rc;
	rc.type = RT_Quad;
	rc.color = color;
	rc.z_index = z_index;
	rc.x0 = position.x - (size.x / 2.f);
	rc.y0 = position.y + (size.y / 2.f);
	rc.x1 = rc.x0 + size.x;
	rc.y1 = rc.y1 - size.y;
	commands.push(rc);
}

static void push_border_quad(f32 x0, f32 y0, f32 x1, f32 y1, f32 thickness, const ch::Color& color, f32 z_index = 9.f) {
	Render_Command rc;
	rc.type = RT_Border_Quad;
	rc.color = color;
	rc.z_index = z_index;
	rc.x0 = x0;
	rc.y0 = y0;
	rc.x1 = x1;
	rc.y1 = y1;
	rc.thickness = thickness;
	commands.push(rc);
}

static void push_border_quad(ch::Vector2 position, ch::Vector2 size, f32 thickness, const ch::Color& color, f32 z_index = 9.f) {
	Render_Command rc;
	rc.type = RT_Border_Quad;
	rc.color = color;
	rc.z_index = z_index;
	rc.x0 = position.x - (size.x / 2.f);
	rc.y0 = position.y + (size.y / 2.f);
	rc.x1 = rc.x0 + size.x;
	rc.y1 = rc.y1 - size.y;
	rc.thickness = thickness;
	commands.push(rc);
}

static void push_text(const ch::String& text, f32 x, f32 y, const ch::Color& color, f32 z_index = 9.f) {
	Render_Command rc;
	rc.type = RT_Text;
	rc.color = color;
	rc.z_index = z_index;
	rc.text = text;
	rc.x = x;
	rc.y = y - font.line_gap;
	commands.push(rc);
}

static void push_text(const char* tstr, f32 x, f32 y, const ch::Color& color, f32 z_index = 9.f) {
	Render_Command rc;
	rc.type = RT_Text;
	rc.color = color;
	rc.z_index = z_index;
	rc.text = ch::String(tstr);
	rc.x = x;
	rc.y = y - font.line_gap;
	commands.push(rc);
}

void gui_text(const ch::String& text, f32 x, f32 y, const ch::Color& color) {
	push_text(text, x, y, color);
}

void gui_text(const char* text, f32 x, f32 y, const ch::Color& color) {
	gui_text(ch::String(text), x, y, color);
}

bool gui_button(const ch::String& text, f32 x0, f32 y0, f32 x1, f32 y1) {
	const bool lmb_was_down = was_mouse_button_pressed(CH_MOUSE_LEFT);

	const ch::Vector2 mouse_pos(current_mouse_position.x, -current_mouse_position.y);
	// const AABB button_box(x0, y0, x1, y1);
	const bool mouse_over = false;

	ch::Color color = button_color;

	if (mouse_over) {
		color = hovered_button_color;
	}

	push_quad(x0, y0, x1, y1, color);
	{
		const ch::Vector2 text_draw_size = get_text_size(text);
		const f32 x = ((x1 - x0) / 2.f) - (text_draw_size.x / 2.f);
		const f32 y = y0 + ((y1 - y0) / 2.f);
		push_text(text, x, y, ch::white);
	}

	return mouse_over && lmb_was_down;
}

bool gui_button(const char* text, f32 x0, f32 y0, f32 x1, f32 y1) {
	ch::String s(text);
	return gui_button(s, x0, y0, x1, y1);
}

void draw_gui() {
	for (const Render_Command& it : commands) {
		switch (it.type) {
		case RT_Quad:
			draw_quad(it.x0, it.y0, it.x1, it.y1, it.color, it.z_index);
			break;
		case RT_Border_Quad:
			draw_border_quad(it.x0, it.y0, it.x1, it.y1, it.thickness, it.color, it.z_index);
			break;
		case RT_Text:
			draw_string(it.text, it.x, it.y - FONT_SIZE, it.color, font, it.z_index);
		}
	}

	commands.count = 0;
}

void Vertical_Layout::row() {
	at_y -= row_height;
}
