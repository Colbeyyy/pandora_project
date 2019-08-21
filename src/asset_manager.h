#pragma once

#include <ch_stl/filesystem.h>

struct Shader;
struct Texture;

void init_am();
void refresh_am();

bool load_asset(const ch::Path& path, ch::File_Data* fd);
Shader* find_shader(const tchar* name);

Texture* find_texture(const tchar* name);