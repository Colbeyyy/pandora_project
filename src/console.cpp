#include "console.h"
#include "draw.h"
#include "input.h"
#include "game.h"
#include "font.h"

#include <ch_stl/math.h>
#include <ch_stl/array.h>
#include <ch_stl/time.h>
#include <ch_stl/gap_buffer.h>

#include <stdio.h>
#include <stdarg.h>

enum Console_State {
	CS_Closed,
	CS_Small,
	CS_Full,
	CS_MAX
};

static u32 console_state;
static f32 current_height;
static f32 target_height;

static Shader* shape_shader;
static Shader* text_shader;

static const f32 open_speed = 10.f;

static ch::Color background_color = 0x052329FF;
static ch::Color foreground_color = 0xd6b58dFF;
static ch::Color cursor_color     = 0x81E38EFF;
static ch::Color selection_color  = 0x000EFFFF;
static f32 y_padding = 5.f;
static f32 x_padding = 5.f;

static const usize message_size = 2048;

struct Console_Entry {
	bool is_log_entry = false;
	ch::Date_Time time_created;
	Log_Severity severity;
	tchar message[message_size];

	Console_Entry() : is_log_entry(false), time_created(ch::get_local_time()) {}
	Console_Entry(Log_Severity _severity) : is_log_entry(true), time_created(ch::get_local_time()), severity(_severity) {}
};

ch::Array<Console_Entry> console_entries = ch::get_heap_allocator();
static ch::Gap_Buffer<tchar> the_command_buffer;
static usize cursor = 0;
static usize selection = 0;

static bool has_selection() {
	return cursor != selection;
}

static void remove_selection() {
	assert(has_selection());

	if (cursor > selection) {
		for (usize i = cursor; i > selection; i--) {
			the_command_buffer.remove_at_index(i);
		}
		cursor = selection;
	} else {
		for (usize i = selection; i > cursor; i--) {
			if (i < the_command_buffer.count()) {
				the_command_buffer.remove_at_index(i);
			}
		}
		selection = cursor;
	}
}

static void add_char_to_console_buffer(tchar c) {
	if (has_selection()) remove_selection();

	the_command_buffer.insert(c, cursor);
	cursor += 1;
	selection += 1;
}

static void reset_console_buffer() {
	the_command_buffer.gap = the_command_buffer.data;
	the_command_buffer.gap_size = the_command_buffer.allocated;
	the_command_buffer.insert(0, 0);
	cursor = 0;
	selection = 0;
}

static void console_input(void* owner, Input_Event* event) {
	const u8 c = event->c;

	if (console_state == CS_Closed && c != '`') return;

	const bool shift_down = is_key_down(CH_KEY_SHIFT);
	const bool ctrl_down = is_key_down(CH_KEY_CONTROL);

	if (event->type == ET_Char_Entered) {
		if (c < 32) return;
		switch(c) {
			case '`': {
				console_state += 1;
				if (console_state == Console_State::CS_MAX) console_state = 0;

				switch (console_state) {
				case Console_State::CS_Closed:
					target_height = 0.f;
					break;
				case Console_State::CS_Small:
					target_height = FONT_SIZE;
					break;
				case Console_State::CS_Full:
					target_height = 600.f;
					break;
				case Console_State::CS_MAX:
					invalid_code_path;
					break;
				}
				return;
			} break;
			case ' ':
				add_char_to_console_buffer(' ');
				break;
			default:
				if (ch::is_whitespace(c)) return;
				add_char_to_console_buffer(c);
				break;
		}
	} else if (event->type == ET_Key_Pressed) {
		switch (c) {
			case CH_KEY_LEFT:
				if (cursor > 0) {
					cursor -= 1;
					if (!shift_down) selection = cursor;
				}
				break;
			case CH_KEY_RIGHT:
				if (cursor < the_command_buffer.count() - 1) {
					cursor += 1;
					if (!shift_down) selection = cursor;
				}
				break;
			case CH_KEY_BACKSPACE:
				if (cursor > 0) {
					if (has_selection()) remove_selection();
					else {
						the_command_buffer.remove_at_index(cursor);
						cursor -= 1;
						selection -= 1;
					}
				}
				break;
			case CH_KEY_DELETE:
				if (cursor < the_command_buffer.count() - 1) {
					if (has_selection()) remove_selection();
					else {
						the_command_buffer.remove_at_index(cursor + 1);
					}
				}
				break;
			case CH_KEY_ENTER: {
				if (the_command_buffer.count() > 0) {
					Console_Entry it;

					bool found_char = false;
					for (usize i = 0; i < the_command_buffer.count() - 1; i++) {
						tchar tcb = the_command_buffer[i];
						it.message[i] = tcb;
						if (ch::is_letter(tcb)) found_char = true;
					}
					it.message[the_command_buffer.count() - 1] = 0;

					reset_console_buffer();

					// @NOTE(CHall): if we found a char then push
					if (found_char) {
						console_entries.push(it);
					}
				}
			} break;
		}
	}
}

void init_console() {
	shape_shader = find_shader(CH_TEXT("solid_shape"));
	text_shader = find_shader(CH_TEXT("font"));
	console_entries.reserve(1024);
	the_command_buffer = ch::Gap_Buffer<tchar>(2048, ch::get_heap_allocator());
	reset_console_buffer();

	bind_event_listener(Event_Listener(nullptr, console_input, ET_None));
}

void tick_console(f32 dt) {
	current_height = ch::interp_to(current_height, target_height, dt, open_speed);
}

void draw_console() {
	assert(shape_shader);
	shape_shader->bind();
	const ch::Vector2 viewport_size = the_window.get_viewport_size();
	const f32 width = (f32)viewport_size.ux;

	render_right_handed();

	const f32 font_height = FONT_SIZE;

	imm_begin();
	// @NOTE(CHall): Background
	{
		const f32 x0 = 0.f;
		const f32 y0 = 0.f;
		const f32 x1 = x0 + width;
		const f32 y1 = y0 - current_height;
		imm_quad(x0, y0, x1, y1, background_color);
	}
	// @NOTE(CHall): Command Bar
	const f32 bar_height = font_height;
	{
		const f32 x0 = 0.f;
		const f32 y0 = -current_height + bar_height;
		const f32 x1 = x0 + width;
		const f32 y1 = y0 - bar_height;
		imm_quad(x0, y0, x1, y1, foreground_color);
	}
	imm_flush();
	assert(text_shader);
	text_shader->bind();
	font.bind();
	refresh_transform();
	imm_begin();
	// @NOTE(CHall): History Text
	{
		const f32 x = x_padding;
		f32 y = -current_height + bar_height + y_padding * 2.f;
		for (usize i = console_entries.count - 1; i < -1; i -= 1) {
			const Console_Entry& it = console_entries[i];
			const ch::Color color = it.is_log_entry ? it.severity : foreground_color;
			imm_string(it.message, x, y, color, font);
			y += bar_height + y_padding;
			if (y > 0.f) break;
		}
	}
	// @NOTE(CHall): Command Text
	{
		auto draw_rect_at_char = [](f32 x, f32 y, const Font_Glyph& g, const ch::Color& color) {
			imm_flush();
			const f32 x0 = x;
			const f32 y0 = y + font.ascent;
			const f32 x1 = x0 + g.advance;
			const f32 y1 = y + font.descent;
			draw_quad(x0, y0, x1, y1, color);
			imm_begin();
			font.bind();
		};

		f32 x = x_padding;
		f32 y = -current_height + bar_height - font.ascent;
		for(usize i = 0; i < the_command_buffer.count(); i++) {
			const tchar c = the_command_buffer[i];
			const Font_Glyph& glyph = c == ch::eos ? font[' '] : font[c];

			const bool is_in_selection = (cursor > selection && i >= selection && i < cursor) || (cursor < selection && i < selection && i >= cursor);
			if (is_in_selection) {
				draw_rect_at_char(x, y, glyph, selection_color);
			}

			const bool is_in_cursor = i == cursor;
			if (is_in_cursor) {
				draw_rect_at_char(x, y, glyph, cursor_color);
			}

			const ch::Color color = (is_in_selection && !is_in_cursor) ? ch::white : background_color;
			imm_glyph(glyph, x, y, color, font);
			x += glyph.advance;
		}

		if (cursor == the_command_buffer.count()) {
			draw_rect_at_char(x, y, font[' '], cursor_color);
		}
	}
	imm_flush();
}

void output_log(Log_Severity severity, const tchar* fmt, ...) {
	Console_Entry it(severity);

	const u32 hour = it.time_created.hour > 12 ? it.time_created.hour - 12 : it.time_created.hour;
	const usize offset = sprintf(it.message, CH_TEXT("[%lu:%lu:%lu]"), hour, it.time_created.minute, it.time_created.second);

	va_list args;
	va_start(args, fmt);
#if CH_PLATFORM_WINDOWS
	vsprintf(it.message + offset, fmt, args);
#else
	#error need own sprintf
#endif
	va_end(args);
	console_entries.push(it);
}

void console_log(const tchar* fmt, ...) {
	Console_Entry it;

	va_list args;
	va_start(args, fmt);
#if CH_PLATFORM_WINDOWS
	vsprintf(it.message, fmt, args);
#else
#error need own sprintf
#endif
	va_end(args);
	console_entries.push(it);
}
