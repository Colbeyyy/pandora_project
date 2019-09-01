#include "input.h"
#include "game.h"

#include <ch_stl/array.h>

ch::Vector2 current_mouse_position;
static bool exit_requested = false;
static bool keys_down[255];
static bool keys_pressed[255];

static bool mb_down[3];
static bool mb_pressed[3];

ch::Array<Event_Listener> listeners = ch::get_heap_allocator();

static Input_Event make_window_resize_event(u32 old_width, u32 old_height) {
	Input_Event result = {};
	result.type = ET_Window_Resize;
	result.old_width = old_width;
	result.old_height = old_height;

	return result;
}

static Input_Event make_exit_requested_event() {
	Input_Event result = {};
	result.type = ET_Exit_Requested;
	return result;
}

static Input_Event make_mouse_down_event(ch::Vector2 mouse_position) {
	Input_Event result = {};
	result.type = ET_Mouse_Down;
	result.mouse_position = mouse_position;
	return result;
}

static Input_Event make_mouse_up_event(ch::Vector2 mouse_position) {
	Input_Event result = {};
	result.type = ET_Mouse_Up;
	result.mouse_position = mouse_position;
	return result;
}

static Input_Event make_mouse_moved_event(ch::Vector2 mouse_position) {
	Input_Event result = {};
	result.type = ET_Mouse_Moved;
	result.mouse_position = mouse_position;
	return result;
}

static Input_Event make_mouse_wheel_scrolled_event(f32 delta) {
	Input_Event result = {};
	result.type = ET_Mouse_Wheel_Scrolled;
	result.delta = delta;
	return result;
}

static Input_Event make_key_pressed_event(u8 key_code) {
	Input_Event result = {};
	result.type = ET_Key_Pressed;
	result.key_code = key_code;
	return result;
}


static Input_Event make_key_released_event(u8 key_code) {
	Input_Event result = {};
	result.type = ET_Key_Released;
	result.key_code = key_code;
	return result;
}

static Input_Event make_char_entered_event(u32 c) {
	Input_Event result = {};
	result.type = ET_Char_Entered;
	result.c = c;
	return result;
}

Event_Listener make_event_listener(void* owner, Process_Event process_event, Event_Type type) {
	Event_Listener result = {};
	result.owner = owner;
	result.process_func = process_event;
	result.type = type;
	assert(result);
	return result;
}

static void process_input_event(Input_Event* event) {
	for (Event_Listener& el : listeners) {
		if (el.type == event->type || el.type == ET_None) {
			el.process_func(el.owner, event);
		}
	}
}

void init_input() {
	the_window.on_exit_requested = [](const ch::Window& window) {
		exit_requested = true;
	};

	the_window.on_key_pressed = [](const ch::Window& window, u8 key) {
		if (!keys_down[key]) {
			keys_pressed[key] = true;
		}
		keys_down[key] = true;
		Input_Event event = make_key_pressed_event(key);
		process_input_event(&event);
	};

	the_window.on_key_released = [](const ch::Window& window, u8 key) {
		keys_down[key] = false;
		Input_Event event = make_key_released_event(key);
		process_input_event(&event);
	};

	the_window.on_mouse_button_down = [](const ch::Window& window, u8 mouse_button) {
		if (!mb_down[mouse_button]) {
			mb_pressed[mouse_button] = true;
		}
		mb_down[mouse_button] = true;
	};

	the_window.on_mouse_button_up = [](const ch::Window& window, u8 mouse_button) {
		mb_down[mouse_button] = false;
	};

	the_window.on_char_entered = [](const ch::Window& window, u32 c) {
		Input_Event event = make_char_entered_event(c);
		process_input_event(&event);
	};

	listeners.reserve(32);
}

void process_input() {
	ch::mem_zero(keys_pressed, sizeof(keys_pressed));
	ch::mem_zero(mb_pressed, sizeof(mb_pressed));

	ch::poll_events();

	ch::Vector2 mouse_pos;
	if (the_window.get_mouse_position(&mouse_pos)) {
		current_mouse_position = ch::Vector2((f32)mouse_pos.ux, (f32)mouse_pos.uy);
	}
	else {
		current_mouse_position = -1.f;
	}

	if (was_key_pressed(CH_KEY_ESCAPE)) {
		exit_requested = true;
	}
}

bool is_key_down(u8 key) {
	return keys_down[key];
}

bool was_key_pressed(u8 key) {
	return keys_pressed[key];
}

bool is_mouse_button_down(u8 mouse_button) {
	return mb_down[mouse_button];
}

bool was_mouse_button_pressed(u8 mouse_button) {
	return mb_pressed[mouse_button];
}

bool is_exit_requested() {
	return exit_requested;
}

bool bind_event_listener(const Event_Listener& event_listener) {
	if (listeners.contains(event_listener)) return false;

	assert(event_listener.process_func);
	listeners.push(event_listener);
	return true;
}

bool unbind_event_listener(const Event_Listener& event_listener) {
	const ssize index = listeners.find(event_listener);
	if (index == -1) return false;
	listeners.remove(index);
	return true;
}