#include "asset.h"
#include "shader.h"
#include "texture.h"

#include <ch_stl/time.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Asset_Manager asset_manager;

static bool set_to_res_path() {
	ch::Path exe_path = ch::get_app_path();

	exe_path.remove_until_directory();
	exe_path.remove_until_directory();
	exe_path.append(CH_TEXT("res"));

	if (!ch::set_current_path(exe_path)) return false;

	return true;
}

void Asset_Manager::init() {
	assert(ch::is_gl_loaded());

	const usize amount = 1024 * 1024 * 128;
	allocator = ch::make_arena_allocator(amount);

	loaded_shaders = ch::Hash_Table<ch::String, Lookup<Shader>>(ch::get_heap_allocator());
	loaded_textures = ch::Hash_Table<ch::String, Lookup<Texture>>(ch::get_heap_allocator());
	   
	set_to_res_path();

	for (ch::Recursive_Directory_Iterator itr; itr.can_advance(); itr.advance()) {
		ch::reset_arena_allocator(&allocator);

		const ch::Directory_Result r = itr.get();

		if (r.type == ch::DRT_File) {
			ch::Path full_path = itr.current_path;
			full_path.append(r.file_name);
			const ch::String ext = full_path.get_extension();

			ch::File_Data fd;
			load_asset(full_path, &fd);
			if (ext == CH_TEXT("glsl")) {
				Shader s;
				if (Shader::load_from_source((const GLchar*)fd.data, &s)) {
					Lookup<Shader> p;
					p.fd = r;
					p.value = s;
					loaded_shaders.push(full_path.get_filename().copy(ch::get_heap_allocator()), p);
				}
			} else if (ext == CH_TEXT("png")) {
				const s32 desired_components = (s32)BT_RGBA;

				stbi_set_flip_vertically_on_load(true);

				Bitmap bm;
				bm.data = stbi_load_from_memory(fd.data, (int)fd.size, &bm.width, &bm.height, &bm.num_components, desired_components);
				defer(stbi_image_free(bm.data));
				if (bm) {
					Texture t(bm);

					Lookup<Texture> p;
					p.fd = r;
					p.value = t;
					loaded_textures.push(full_path.get_filename().copy(ch::get_heap_allocator()), p);
				}
			}
		}
	}
}

void Asset_Manager::refresh() {
	CH_SCOPED_TIMER(asset_manager_refresh);
	set_to_res_path();

	for (ch::Recursive_Directory_Iterator itr; itr.can_advance(); itr.advance()) {
		ch::reset_arena_allocator(&allocator);
		const ch::Directory_Result r = itr.get();

		if (r.type == ch::DRT_File) {
			ch::Path full_path = itr.current_path;
			full_path.append(r.file_name);
			const ch::String ext = full_path.get_extension();

			if (ext == CH_TEXT("glsl")) {
				Lookup<Shader>* ls = loaded_shaders.find(full_path.get_filename().copy(allocator));
				if (ls && ls->fd.last_write_time < r.last_write_time) {
					ch::File_Data fd;
					if (!load_asset(full_path, &fd)) continue;

					Shader s;
					if (Shader::load_from_source((const GLchar*)fd.data, &s)) {
						ls->value.free();
						ls->value = s;
						ls->fd = r;
					}
				}
			} else if (ext == CH_TEXT("png")) {
				Lookup<Texture>* lt = loaded_textures.find(full_path.get_filename().copy(allocator));

				if (lt && lt->fd.last_write_time < r.last_write_time) {
					ch::File_Data fd;
					if (!load_asset(full_path, &fd)) continue;

					const s32 desired_components = (s32)BT_RGBA;
					stbi_set_flip_vertically_on_load(true);
					Bitmap bm;
					bm.data = stbi_load_from_memory(fd.data, (int)fd.size, &bm.width, &bm.height, &bm.num_components, desired_components);
					defer(stbi_image_free(bm.data));
					if (bm) {
						lt->value.free();
						lt->value = Texture(bm);
						lt->fd = r;
					}
				}
			}
		}
	}
}

bool Asset_Manager::load_asset(const tchar* path, ch::File_Data* fd) {
	return ch::load_file_into_memory(path, fd, allocator);
}

Shader* Asset_Manager::find_shader(const tchar* name) {
	ch::String search(name);
	defer(search.free());
	Lookup<Shader>* r = loaded_shaders.find(search);
	if (r) return &r->value;
	return get_default_shader();
}

Shader* Asset_Manager::find_shader(u32 id) {
	return get_default_shader();
}

Texture* Asset_Manager::find_texture(const tchar* name) {
	ch::String search(name);
	defer(search.free());
	Lookup<Texture>* r = loaded_textures.find(search);
	if (r) return &r->value;
	return nullptr;
}