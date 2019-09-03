#pragma once

#include <ch_stl/types.h>
extern bool show_tile_grid;
extern bool show_transform_origin;
extern bool show_collider_debug;

void init_debug();
void tick_debug(f32 dt);
