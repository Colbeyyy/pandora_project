#include <ch_stl/types.h>

#include "../game_state.h"

int main() {
    game_state.init();
    game_state.loop();
    game_state.shut_down();
    return 0;
}