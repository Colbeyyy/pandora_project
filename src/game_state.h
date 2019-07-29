#pragma once

#include <ch_stl/types.h>
#include <ch_stl/window.h>

#include "entity.h"
#include "asset.h"

#include "draw.h"


struct World;

struct Input_State {
	ch::Vector2 current_mouse_position;
    bool exit_requested = false;
	bool keys_down[255];
	bool keys_pressed[255];

	bool mb_down[3];
	bool mb_pressed[3];

	void bind(ch::Window* window);
	void reset();

	CH_FORCEINLINE bool is_key_down(u8 key) const { return keys_down[key]; }
	CH_FORCEINLINE bool was_key_pressed(u8 key) const { return keys_pressed[key]; }

	CH_FORCEINLINE bool is_mouse_button_down(u8 mouse_button) const { return mb_down[mouse_button]; }
	CH_FORCEINLINE bool was_mouse_button_pressed(u8 mouse_button) const { return mb_pressed[mouse_button]; }
};

extern Input_State g_input_state;

struct Game_State {

	Asset_Manager asset_manager;

	ch::Window window;
	World* loaded_world;

	Entity_Id player_id;

	struct Font font;

	f32 dt;
	u32 fps;

    void init();
    void loop();
    void shut_down();

    void process_inputs();
    void tick_game(f32 dt);
    void draw_game();

	void reset_world();

};

extern Game_State g_game_state;