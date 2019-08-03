#include "asset.h"
#include "shader.h"
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

static Asset_Manager g_asset_manager;

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

	const usize amount = 1024 * 1024 * 16;
	allocator = ch::make_arena_allocator(amount);

	loaded_shaders.allocator = ch::get_heap_allocator();
	loaded_textures.allocator = ch::get_heap_allocator();
	   
	set_to_res_path();

	for (ch::Recursive_Directory_Iterator itr; itr.can_advance(); itr.advance()) {
		ch::reset_arena_allocator(&allocator);

		const ch::Directory_Result r = itr.get();

		if (r.type == ch::DRT_File) {
			ch::Path full_path = itr.current_path;
			full_path.append(r.file_name);
			const ch::String ext = full_path.get_extension();

			ch::File_Data fd;
			assert(load_asset(full_path, &fd));
			if (ext == CH_TEXT("glsl")) {
				Shader s;
				if (Shader::load_from_source((const GLchar*)fd.data, &s)) {
					Lookup<Shader> p;
					p.key = r;
					p.value = s;
					loaded_shaders.push(p);
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
					p.key = r;
					p.value = t;
					loaded_textures.push(p);
				}
			}
		}
	}
}

void Asset_Manager::refresh() {
	set_to_res_path();

	for (ch::Recursive_Directory_Iterator itr; itr.can_advance(); itr.advance()) {
		ch::reset_arena_allocator(&allocator);
		const ch::Directory_Result r = itr.get();

		if (r.type == ch::DRT_File) {
			ch::Path full_path = itr.current_path;
			full_path.append(r.file_name);
			const ch::String ext = full_path.get_extension();

			if (ext == CH_TEXT("glsl")) {
				for (Lookup<Shader>& it : loaded_shaders) {
					if (ch::streq(it.key.file_name, r.file_name) && it.key.last_write_time < r.last_write_time) {
						it.value.free();

						ch::File_Data fd;
						assert(load_asset(full_path, &fd));

						Shader s;
						if (Shader::load_from_source((const GLchar*)fd.data, &s)) {
							it.value = s;
							it.key = r;
						}
					}
				}
			} else if (ext == CH_TEXT("png")) {
				for (Lookup<Texture>& it : loaded_textures) {
					if (ch::streq(it.key.file_name, r.file_name) && it.key.last_write_time < r.last_write_time) {
						it.value.free();

						ch::File_Data fd;
						assert(load_asset(full_path, &fd));

						const s32 desired_components = (s32)BT_RGBA;
						stbi_set_flip_vertically_on_load(true);
						Bitmap bm;
						bm.data = stbi_load_from_memory(fd.data, (int)fd.size, &bm.width, &bm.height, &bm.num_components, desired_components);
						defer(stbi_image_free(bm.data));
						if (bm) {
							it.value = Texture(bm);
							it.key = r;
						}
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
	for (Lookup<Shader>& it : loaded_shaders) {
		ch::Path fn = it.key.file_name;
		if (fn.get_filename() == name) {
			return &it.value;
		}
	}

	return nullptr;
}

Shader* Asset_Manager::find_shader(u32 id) {
	for (Lookup<Shader>& it : loaded_shaders) {
		if (it.value.program_id == id) {
			return &it.value;
		}
	}

	return nullptr;
}

Texture* Asset_Manager::find_texture(const tchar* name) {
	for (Lookup<Texture>& it : loaded_textures) {
		ch::Path fn = it.key.file_name;
		if (fn.get_filename() == name) {
			return &it.value;
		}
	}

	return nullptr;
}

Asset_Manager& Asset_Manager::get() {
	return g_asset_manager;
}
