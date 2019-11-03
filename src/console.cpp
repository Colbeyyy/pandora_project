#include "console.h"
#include "draw.h"
#include "input.h"
#include "game.h"
#include "font.h"

#include "console_commands.h"

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

static ch::Color background_color	 = 0x052329FF;
static ch::Color foreground_color	 = 0xd6b58dFF;
static ch::Color cursor_color		 = 0x81E38EFF;
static ch::Color selection_color	 = 0x000EFFFF;
static ch::Color selected_text_color = ch::white;
static f32 y_padding = 5.f;
static f32 x_padding = 5.f;

static const usize message_size = 2048;

struct Console_Entry {
	bool is_log_entry = false;
	ch::Date_Time time_created;
	Log_Severity severity;
	char message[message_size];

	Console_Entry() : is_log_entry(false), time_created(ch::get_local_time()) {}
	Console_Entry(Log_Severity _severity) : is_log_entry(true), time_created(ch::get_local_time()), severity(_severity) {}
};

ch::Array<Console_Entry> console_entries = ch::get_heap_allocator();
static ch::Gap_Buffer<char> the_command_buffer;
static ssize cursor = 0;
static ssize selection = 0;

f32 cursor_blink_time = 0.f;
bool show_cursor = true;
static bool show_logs = true;

static bool has_selection() {
	return cursor != selection;
}

static void set_cursor(ssize new_cursor) {
	assert(new_cursor == -1 || new_cursor < (ssize)the_command_buffer.count());

	cursor = new_cursor;
	if (!is_key_down(CH_KEY_SHIFT)) {
		selection = cursor;
	}
}

static ssize seek_dir(bool left) {
	ssize result = cursor + (!left * 2);
	if (left) {
		for (; result > -1; result -= 1) {
			const char c = the_command_buffer[result];
			if (ch::is_symbol(c) || ch::is_whitespace(c)) {
				result -= 1;
				break;
			}
		}
	} else {
		for (; result < (ssize)the_command_buffer.count() - 1; result += 1) {
			const char c = the_command_buffer[result];
			if (ch::is_symbol(c) || ch::is_whitespace(c)) {
				result -= 1;
				break;
			}
		}
	}

	return result;
}

static void remove_selection() {
	assert(has_selection());

	if (cursor > selection) {
		for (ssize i = cursor; i > selection; i--) {
			the_command_buffer.remove_at_index(i);
		}
		cursor = selection;
	} else {
		for (ssize i = selection; i > cursor; i--) {
			if (i < (ssize)the_command_buffer.count()) {
				the_command_buffer.remove_at_index(i);
			}
		}
		selection = cursor;
	}
}

static void add_char_to_command_buffer(char c) {
	if (has_selection()) remove_selection();

	the_command_buffer.insert(c, cursor + 1);
	cursor += 1;
	selection += 1;
}

static void reset_command_buffer() {
	the_command_buffer.gap = the_command_buffer.data;
	the_command_buffer.gap_size = the_command_buffer.allocated;
	cursor = -1;
	selection = -1;
}

static void set_console_state(u32 new_cs) {
	if (new_cs == Console_State::CS_MAX) new_cs = 0;
	console_state = new_cs;

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
}

static void process_command(Console_Entry* entry) {
	const usize message_count = ch::strlen(entry->message);
	assert(message_count > 0);

	ch::String command;
	command.data = entry->message;
	command.count = message_count;
	command.eat_whitespace();

	const ssize space_index = command.find_from_left(' ');
	ch::String command_name = command;
	ch::String params = command;

	if (space_index == -1) {
		params.count = 0;
	} else {
		params.advance(space_index);
		params.eat_whitespace();
		command_name.count = space_index;
	}

#define FIND_COMMAND(func, name, help) if (command_name == name) { if (func(params)) set_console_state(CS_Full); else set_console_state(CS_Closed); return; } 
	CONSOLE_COMMANDS(FIND_COMMAND);
#undef FIND_COMMAND

	console_log("Command not found \"%.*s\"", command_name.count, command_name.data);
	set_console_state(CS_Full);
}

static void console_input(void* owner, Input_Event* event) {
	const u8 c = event->c;

	if (console_state == CS_Closed && c != '`') return;

	const bool shift_down = is_key_down(CH_KEY_SHIFT);
	const bool ctrl_down = is_key_down(CH_KEY_CONTROL);

	if (event->type == ET_Char_Entered) {
		if (c < 32 || c > 126) return;
		switch(c) {
			case '`': 
				set_console_state(console_state + 1);
				return;
			case ' ':
				add_char_to_command_buffer(' ');
				break;
			default:
				if (ch::is_whitespace(c)) return;
				add_char_to_command_buffer(c);
				break;
		}
	} else if (event->type == ET_Key_Pressed) {
		switch (c) {
			case CH_KEY_LEFT:
				if (cursor > -1) {
					const ssize new_cursor = ctrl_down ? seek_dir(true) : cursor - 1;
					set_cursor(new_cursor);
				}
				break;
			case CH_KEY_RIGHT:
				if (cursor < (ssize)the_command_buffer.count() - 1) {
					const ssize new_cursor = ctrl_down ? seek_dir(false) : cursor + 1;
					set_cursor(new_cursor);
				}
				break;
			case CH_KEY_HOME:
				set_cursor(-1);
				break;
			case CH_KEY_END:
				set_cursor(the_command_buffer.count() - 1);
				break;
			case CH_KEY_BACKSPACE:
				if (cursor > -1 || has_selection()) {
					if (has_selection()) remove_selection();
					else {
						if (ctrl_down) {
							cursor = seek_dir(true);
							if (has_selection()) remove_selection();
						} else {
							the_command_buffer.remove_at_index(cursor);
							cursor -= 1;
							selection -= 1;
						}
					}
				}
				break;
			case CH_KEY_DELETE:
				if (cursor < (ssize)the_command_buffer.count() - 1 || has_selection()) {
					if (has_selection()) remove_selection();
					else {
						if (ctrl_down) {
							cursor = seek_dir(false);
							if (has_selection()) remove_selection();
						} else {
							the_command_buffer.remove_at_index(cursor + 1);
						}
					}
				}
				break;
			case CH_KEY_ENTER: {
				Console_Entry it;

				bool found_char = false;
				for (usize i = 0; i < the_command_buffer.count(); i++) {
					char tcb = the_command_buffer[i];
					it.message[i] = tcb;
					if (ch::is_letter(tcb)) found_char = true;
				}
				it.message[the_command_buffer.count()] = 0;

				reset_command_buffer();

				console_entries.push(it);
				// @NOTE(CHall): if we found a char then process
				if (found_char) {
					process_command(&it);
				} else {
					set_console_state(CS_Full);
				}
			} break;
			case 'V': {
				if (!ctrl_down) break;
				ch::String clipboard;
				defer(clipboard.free());
				if (ch::copy_from_clipboard(the_window, &clipboard)) {
					for (usize i = 0; i < clipboard.count; i++) {
						const char c = clipboard[i];
						if (c == '\n' || c == '\r' || c == ch::eos || c == '\t') continue;
						add_char_to_command_buffer(c);
					}
				}
			} break;
			case 'C':
				if (!ctrl_down) break;
				output_log(LS_Warning, "Sorry! Copying from is the command buffer is currently not supported");
				break;
			case 'X':
				if (!ctrl_down) break;
				output_log(LS_Warning, "Sorry! Copying from is the command buffer is currently not supported");
				reset_command_buffer();
				break;
		}
	}

	show_cursor = true;
	cursor_blink_time = 0.f;
}

void init_console() {
	shape_shader = find_shader("solid_shape");
	text_shader = find_shader("font");
	console_entries.reserve(1024);
	the_command_buffer = ch::Gap_Buffer<char>(2048, ch::get_heap_allocator());
	reset_command_buffer();

	bind_event_listener(Event_Listener(nullptr, console_input, ET_None));
}

void tick_console(f32 dt) {
	current_height = ch::interp_to(current_height, target_height, dt, open_speed);
	
	u32 blink_time;
	if (!ch::get_caret_blink_time(&blink_time)) {
		cursor_blink_time = 0.f;
		show_cursor = true;
	}
	cursor_blink_time += dt;
	if (cursor_blink_time > (f32)blink_time / 1000.f) {
		show_cursor = !show_cursor;
		cursor_blink_time = 0.f;
	}
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
			f32 offset_x = 0.f;
			
			if (!it.is_log_entry) {
				offset_x += imm_string("> ", x, y, color, font).x;
			} else if (!show_logs) continue;
			
			imm_string(it.message, x + offset_x, y, color, font);
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
		x += imm_string("> ", x, y, background_color, font).x;

		for(usize i = 0; i < the_command_buffer.count(); i++) {
			const char c = the_command_buffer[i];
			const Font_Glyph& glyph = c == ch::eos ? font[' '] : font[c];

			const bool is_in_selection = (cursor > selection && (ssize)i >= selection + 1 && (ssize)i < cursor + 1) || (cursor < selection && (ssize)i < selection + 1 && (ssize)i >= cursor + 1);
			if (is_in_selection) {
				draw_rect_at_char(x, y, glyph, selection_color);
			}

			const bool is_in_cursor = i == cursor + 1 && show_cursor;
			if (is_in_cursor) {
				draw_rect_at_char(x, y, glyph, cursor_color);
			}

			const ch::Color color = (is_in_selection && !is_in_cursor) ? selected_text_color : background_color;
			imm_glyph(glyph, x, y, color, font);
			x += glyph.advance;
		}

		if (cursor + 1 == the_command_buffer.count() && show_cursor) draw_rect_at_char(x, y, font[' '], cursor_color);
	}
	imm_flush();
}

void output_log(Log_Severity severity, const char* fmt, ...) {
	Console_Entry it(severity);

	const u32 hour = it.time_created.hour > 12 ? it.time_created.hour - 12 : it.time_created.hour;
	const usize offset = ch::sprintf(it.message, "[%lu:%lu:%lu]", hour, it.time_created.minute, it.time_created.second);

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

void console_log(const char* fmt, ...) {
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

bool help_command(const ch::String& params) {

#define FIND_COMMAND(func, name, help) if (name != "help") console_log("%s : %s", name, help);
	CONSOLE_COMMANDS(FIND_COMMAND);
#undef FIND_COMMAND

	return true;
}

bool output_log_command(const ch::String& params) {
	if (!params.count) {
		console_log("log command requires some text following it. Example: \"log hello world\"");
		return true;
	}

	o_log("%.*s", params.count, params.data);
	return true;
}

bool clear_command(const ch::String& params) {
	if (params.count) {
		console_log("clear should not have any params");
		return true;
	}
	console_entries.count = 0;
	return true;
}

bool toggle_show_logs(const ch::String& params) {
	if (params.count) {
		console_log("toggle_show_logs should not have any params");
		return true;
	}

	show_logs = !show_logs;
	return true;
}

bool set_show_logs(const ch::String& params) {
	if (!params.count) {
		console_log("set_show_logs requires one single param of either true or false");
		console_log("Example: \"set_show_logs false\"");
		return true;
	}

	ch::String lower_case = params;
	lower_case.to_lowercase();

	if (lower_case == "true" || lower_case == "1") {
		show_logs = true;
	} else if (lower_case == "false" || lower_case == "0") {
		show_logs = false;
	} else {
		console_log("unrecognized param \"%.*s\"", params.count, params.data);
	}

	return true;
}
