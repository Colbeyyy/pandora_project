#pragma once

#include <ch_stl/ch_math.h>
#include <ch_stl/ch_opengl.h>

void draw_init();
void draw_frame_begin();
void draw_frame_end();

void refresh_transform();
void render_right_handed();

void imm_begin();
void imm_flush();

void imm_vertex(f32 x, f32 y, const ch::Vector4& color, ch::Vector2 uv, f32 z_index = 0.f);
void imm_quad(f32 x0, f32 y0, f32 x1, f32 y1, const ch::Vector4& color, f32 z_index = 0.f);