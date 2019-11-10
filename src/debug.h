#pragma once

#include <ch_stl/types.h>
#include <ch_stl/string.h>

extern bool show_tile_grid;

void init_debug();
void tick_debug(f32 dt);

bool toggle_fps(const ch::String& params);
