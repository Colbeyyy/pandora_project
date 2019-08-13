#include <ch_stl/types.h>

#include "../game_state.h"

#if BUILD_DEBUG
int main() {
#else
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif
    game_state.init();
    game_state.loop();
    game_state.shut_down();
    return 0;
}