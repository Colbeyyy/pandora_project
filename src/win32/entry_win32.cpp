#include <ch_stl/types.h>

#include "../game_state.h"

int WinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
    Game_State::get().init();
    Game_State::get().loop();
    Game_State::get().shut_down();
    return 0;
}