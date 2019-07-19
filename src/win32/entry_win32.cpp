#include <ch_stl/types.h>

#include "../game_state.h"

#if BUILD_DEBUG 
#define MAIN_FUNCTION(...) main(int argc, char** argv)
#else
#define MAIN_FUNCTION(...) WinMain(void*, void*, tchar*, int)
#endif

int MAIN_FUNCTION() {
    g_game_state.init();
    g_game_state.loop();
    g_game_state.shut_down();
    return 0;
}