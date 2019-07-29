#include <ch_stl/types.h>

#include "../game_state.h"

int main() {
    g_game_state.init();
    g_game_state.loop();
    g_game_state.shut_down();
    return 0;
}