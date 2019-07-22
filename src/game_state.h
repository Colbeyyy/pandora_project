#pragma once

#include <ch_stl/types.h>
#include <ch_stl/window.h>

#include "entity.h"


struct World;

struct Input_State {
	ch::Vector2 current_mouse_position;
    bool exit_requested = false;
	bool keys_down[255];
	bool keys_pressed[255];

	void bind(ch::Window* window);

	CH_FORCEINLINE bool is_key_down(u8 key) { return keys_down[key]; }
	CH_FORCEINLINE bool was_key_pressed(u8 key) { return keys_pressed[key]; }
};

extern Input_State g_input_state;

struct Game_State {

    f32 delta_time;
	ch::Window window;
	World* loaded_world;

	Entity_Id player_id;

	f32 dt;

    void init();
    void loop();
    void shut_down();

    void process_inputs();
    void tick_game(f32 dt);
    void draw_game();

	void reset_world();

};

extern Game_State g_game_state;