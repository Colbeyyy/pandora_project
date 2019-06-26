#include <ch_stl/ch_stl.h>
#include <ch_stl/ch_filesystem.h>
#include <ch_stl/ch_window.h>
#include <ch_stl/ch_opengl.h>
#include <ch_stl/ch_defer.h>

#if BUILD_DEBUG 
#define MAIN_FUNCTION(...) main(int argc, char** argv)
#else
#define MAIN_FUNCTION(...) WinMain(void*, void*, tchar*, int)
#endif

ch::Window window;

bool exit_requested = false;

int MAIN_FUNCTION() {
    assert(ch::load_gl());
    {
        const u32 width = 1280;
        const u32 height = (u32)((f32)width * (9.f / 16.f));
        assert(ch::create_gl_window(CH_TEXT("pandora project"), width, height, 0, &window));
    }
    assert(ch::make_current(window));

    window.on_exit_requested = [](const ch::Window& window) {
        exit_requested = true;
    };

    window.set_visibility(true);

    while (!exit_requested) {
        ch::poll_events();

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.f, 0.f, 0.5f, 1.f);

        ch::swap_buffers(window);
    }
}