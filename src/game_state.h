#pragma once

#include <ch_stl/types.h>
#include <ch_stl/window.h>

struct Game_State {

    bool exit_requested = false;
    f32 delta_time;
	ch::Window window;

    void init();
    void loop();
    void shut_down();

    void process_inputs();
    void tick_game(f32 dt);
    void draw_game();

};

extern Game_State g_game_state;