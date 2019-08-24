#include "console.h"
#include "draw.h"
#include "input.h"
#include "game.h"
#include "font.h"

#include <ch_stl/math.h>
#include <ch_stl/array.h>
#include <ch_stl/time.h>

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
};

ch::Array<Console_Entry> console_entries = ch::get_heap_allocator();

struct Gap_Buffer {
	tchar* data;
	usize allocated;
	tchar* gap;
	usize gap_size;

	static usize default_gap_size;

	CH_FORCEINLINE usize count() const { return allocated - gap_size; }

	CH_FORCEINLINE tchar& operator[](usize index) {
		assert(index < count());

		if (data + index < gap) {
			return data[index];
		} else {
			return data[index + gap_size];
		}
	}

	CH_FORCEINLINE tchar operator[](usize index) const {
		assert(index < count());

		if (data + index < gap) {
			return data[index];
		}
		else {
			return data[index + gap_size];
		}
	}

	Gap_Buffer() = default;
	Gap_Buffer(usize size);
	void free();
	void resize(usize new_gap_size);
	void move_gap_to_index(usize index);
	tchar* get_index_as_cursor(usize index);
	void add_char(tchar c, usize index);
	void remove_at_index(usize index);
	void remove_between(usize index, usize count);
};

usize Gap_Buffer::default_gap_size = 512;

static Gap_Buffer the_command_buffer;
static usize cursor = 0;
static usize selection = 0;

Gap_Buffer::Gap_Buffer(usize size) {
	if (size < default_gap_size) size = default_gap_size;
	allocated = size;
	data = ch_new tchar[allocated];
	assert(data);

	gap = data;
	gap_size = size;
}

void Gap_Buffer::free() { 
	if (data) ch_delete data;
}

void Gap_Buffer::resize(usize new_gap_size) {
	if (!data) {
		*this = Gap_Buffer(new_gap_size);
		return;
	}
	assert(data);
	assert(gap_size == 0);

	const usize old_size = allocated;
	const usize new_size = old_size + new_gap_size;

	tchar* old_data = data;
	tchar* new_data = (tchar*)ch::realloc(old_data, new_size * sizeof(tchar));
	assert(!new_data);

	data = new_data;
	allocated = new_size;
	gap = (data + old_size);
	gap_size = new_gap_size;
}

void Gap_Buffer::move_gap_to_index(usize index) {
	assert(index < count());

	tchar* index_ptr = &(*this)[index];

	if (index_ptr < gap) {
		const usize amount_to_move = gap - index_ptr;

		ch::mem_copy(gap + gap_size - amount_to_move, index_ptr, amount_to_move * sizeof(tchar));

		gap = index_ptr;
	}
	else {
		const usize amount_to_move = index_ptr - (gap + gap_size);

		memcpy(gap, gap + gap_size, amount_to_move * sizeof(tchar));

		gap += amount_to_move;
	}
}

tchar* Gap_Buffer::get_index_as_cursor(usize index) {
	if (data + index <= gap) {
		return data + index;
	}

	return data + gap_size + index;
}

void Gap_Buffer::add_char(tchar c, usize index) {
	if (gap_size <= 0) {
		resize(default_gap_size);
	}

	tchar* cursor = get_index_as_cursor(index);

	if (cursor != gap) move_gap_to_index(index);

	*cursor = c;
	gap += 1;
	gap_size -= 1;
}

void Gap_Buffer::remove_at_index(usize index) {
	const usize buffer_count = count();
	assert(index < buffer_count);

	tchar* cursor = get_index_as_cursor(index);
	if (cursor == data) {
		return;
	}

	move_gap_to_index(index);

	gap -= 1;
	gap_size += 1;
}

void Gap_Buffer::remove_between(usize index, usize count) {
	// @SPEED(CHall): this is slow
	for (usize i = index; i < index + count; i++) {
		remove_at_index(i);
	}
}

static void add_char_to_console_buffer(tchar c) {
	the_command_buffer.add_char(c, cursor);
	cursor += 1;
	selection += 1;
}

static void console_input(void* owner, Input_Event* event) {
	const u8 c = event->c;

	if (console_state == CS_Closed && c != '`') return;

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
		case CH_KEY_ENTER: {
			if (the_command_buffer.count() > 1) {
				Console_Entry it;

				bool found_char = false;
				for (usize i = 0; i < the_command_buffer.count() - 1; i++) {
					tchar tcb = the_command_buffer[i];
					it.message[i] = tcb;
					if (ch::is_letter(tcb)) found_char = true;
				}
				it.message[the_command_buffer.count() - 1] = 0;
				it.time_created = ch::get_local_time();

				// @NOTE(CHall): Reset buffer
				the_command_buffer.gap = the_command_buffer.data;
				the_command_buffer.gap_size = the_command_buffer.allocated;
				cursor = 0;
				add_char_to_console_buffer(0);
				cursor = 0;

				// @NOTE(Chall): if we found a char then push
				if (found_char) {
					console_entries.push(it);
				}
			}
		} break;
		case ' ':
			add_char_to_console_buffer(' ');
			break;
		default:
			if (ch::is_whitespace(c)) return;
			add_char_to_console_buffer(c);
			break;
	}
}

void init_console() {
	shape_shader = find_shader(CH_TEXT("solid_shape"));
	text_shader = find_shader(CH_TEXT("font"));
	console_entries.reserve(1024);
	the_command_buffer = Gap_Buffer(2048);
	add_char_to_console_buffer(0);
	cursor = 0;

	bind_event_listener(Event_Listener(nullptr, console_input, ET_Char_Entered));

	log(LS_Verbose, CH_TEXT("error"));
	log(LS_Warning, CH_TEXT("fuck"));
	log(LS_Error, CH_TEXT("shit"));
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
			imm_string(it.message, x, y, it.severity, font);
			y += bar_height + y_padding;
		}
	}
	// @NOTE(CHall): Command Text
	{
		f32 x = x_padding;
		f32 y = -current_height + bar_height - font.ascent;
		for(usize i = 0; i < the_command_buffer.count(); i++) {
			const tchar c = the_command_buffer[i];
			const Font_Glyph& glyph = c == ch::eos ? font[' '] : font[c];
			if (i == cursor) {
				imm_flush();
				const f32 x0 = x;
				const f32 y0 = y + font.ascent;
				const f32 x1 = x0 + glyph.advance;
				const f32 y1 = y + font.descent;
				draw_quad(x0, y0, x1, y1, cursor_color);
				imm_begin();
				font.bind();
			}
			imm_glyph(glyph, x, y, background_color, font);
			x += glyph.advance;
		}
	}
	imm_flush();
}

void log(Log_Severity severity, const tchar* fmt, ...) {
	Console_Entry it;
	it.is_log_entry = true;
	it.time_created = ch::get_local_time();
	it.severity = severity;

	const u32 hour = it.time_created.hour > 12 ? it.time_created.hour - 12 : it.time_created.hour;
	const usize offset = sprintf(it.message, CH_TEXT("[%lu:%lu:%lu]"), hour, it.time_created.minute, it.time_created.second);

	va_list args;
	va_start(args, fmt);
	sprintf(it.message + offset, fmt, args);
	va_end(args);
	console_entries.push(it);
}

void log(const tchar* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log(LS_Verbose, fmt, args);
	va_end(args);
}

void log_verbose(const tchar* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log(LS_Verbose, fmt, args);
	va_end(args);
}

void log_warning(const tchar* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log(LS_Warning, fmt, args);
	va_end(args);
}

void log_error(const tchar* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log(LS_Error, fmt, args);
	va_end(args);
}
