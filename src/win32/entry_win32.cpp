#include <ch_stl/types.h>

#include "../game_state.h"

int WinMain(HINSTANCE, HINSTANCE, tchar*, int) {
    g_game_state.init();
    g_game_state.loop();
    g_game_state.shut_down();
    return 0;
}