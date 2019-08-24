#pragma once

#include <ch_stl/math.h>
#include <ch_stl/input.h>

extern ch::Vector2 current_mouse_position;

enum Event_Type {
	ET_None,
	ET_Window_Resize,
	ET_Exit_Requested,
	ET_Mouse_Down,
	ET_Mouse_Up,
	ET_Mouse_Moved,
	ET_Mouse_Wheel_Scrolled,
	ET_Key_Pressed,
	ET_Key_Released,
	ET_Char_Entered,
};

struct Input_Event {
	Event_Type type;

	union {
		struct {
			u32 old_width;
			u32 old_height;
		};
		ch::Vector2 mouse_position;
		f32 delta;
		u32 key_code;
		u8 c;
	};

	bool handled;
	Input_Event() = default;
};

using Process_Event = void(*)(void* owner, Input_Event* event);

struct Event_Listener {
	void* owner;
	Process_Event process_func;
	Event_Type type;

	CH_FORCEINLINE operator bool() const { return owner && process_func; }
	CH_FORCEINLINE bool operator==(const Event_Listener& right) const { return owner == right.owner && process_func == right.process_func; }

	Event_Listener() = default;
	Event_Listener(void* _owner, Process_Event _process_func, Event_Type _type) : owner(_owner), process_func(_process_func), type(_type) {}
};

void init_input();
void process_input();

bool is_key_down(u8 key);
bool was_key_pressed(u8 key);

bool is_mouse_button_down(u8 mouse_button);
bool was_mouse_button_pressed(u8 mouse_button);

bool is_exit_requested();

bool bind_event_listener(const Event_Listener& event_listener);
bool unbind_event_listener(const Event_Listener& event_listener);