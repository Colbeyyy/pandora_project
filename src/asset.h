#pragma once

#include <ch_stl/allocator.h>
#include <ch_stl/array.h>
#include <ch_stl/filesystem.h>
#include <ch_stl/hash_table.h>

template <typename T>
struct Lookup {
	ch::Directory_Result fd;
	T value;
};

struct Shader;
struct Texture;

struct Asset_Manager {
	ch::Allocator allocator;
	ch::Hash_Table<ch::String, Lookup<Shader>> loaded_shaders;
	ch::Hash_Table<ch::String, Lookup<Texture>> loaded_textures;

	Asset_Manager() = default;

	void init();
	void refresh();

	bool load_asset(const tchar* path, ch::File_Data* fd);
	CH_FORCEINLINE usize get_current_size() const { return allocator.get_header<ch::Arena_Allocator_Header>()->current;}

	Shader* find_shader(const tchar* name);
	Shader* find_shader(u32 id);

	Texture* find_texture(const tchar* name);
};

extern Asset_Manager asset_manager;