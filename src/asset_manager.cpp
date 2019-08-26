#include "asset_manager.h"
#include "shader.h"
#include "texture.h"
#include "console.h"

#include <ch_stl/time.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


template <typename T>
struct Lookup {
	ch::Directory_Result key;
	T value;
};

ch::Allocator allocator;

ch::Array<Lookup<Shader*>> loaded_shaders;
ch::Array<Lookup<Texture*>> loaded_textures;

static bool set_to_res_path() {
	ch::Path exe_path = ch::get_app_path();

	exe_path.remove_until_directory();
	exe_path.remove_until_directory();
	exe_path.append(CH_TEXT("res"));

	if (!ch::set_current_path(exe_path)) return false;

	return true;
}

void init_am() {
	assert(ch::is_gl_loaded());

	const usize amount = 1024 * 1024 * 128;
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
			load_asset(full_path, &fd);
			if (ext == CH_TEXT("glsl")) {
				Shader* s = ch_new Shader;
				Lookup<Shader*> p = {};
				p.key = r;
				if (Shader::load_from_source((const GLchar*)fd.data, s)) {
					p.value = s;
					o_log(CH_TEXT("loaded shader %s"), full_path);
				} else {
					o_log_error(CH_TEXT("failed to load shader %s"), full_path);
				}
				loaded_shaders.push(p);
			}
			else if (ext == CH_TEXT("png")) {
				const s32 desired_components = (s32)BT_RGBA;

				stbi_set_flip_vertically_on_load(true);

				Bitmap bm;
				bm.data = stbi_load_from_memory(fd.data, (int)fd.size, &bm.width, &bm.height, &bm.num_components, desired_components);
				defer(stbi_image_free(bm.data));
				Lookup<Texture*> p = {};
				p.key = r;
				if (bm) {
					Texture* t = ch_new Texture(bm);
					p.value = t;
					o_log(CH_TEXT("loaded texture %s"), full_path);
				} else {
					o_log_error(CH_TEXT("failed to load texture %s"), full_path);
				}
				loaded_textures.push(p);
			}
		}
	}
}

void refresh_am() {
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
				for (Lookup<Shader*>& it : loaded_shaders) {
					if (ch::streq(it.key.file_name, r.file_name) && it.key.last_write_time < r.last_write_time) {
						ch::File_Data fd;
						if (!load_asset(full_path, &fd)) continue;

						Shader s;
						if (Shader::load_from_source((const GLchar*)fd.data, &s)) {
							if (it.value) it.value->free();
							*it.value = s;
							o_log(CH_TEXT("shader reloaded %s"), full_path);
						} else {
							o_log_error(CH_TEXT("shader failed to reload %s"), full_path);
						}
						it.key = r;
					}
				}
			}
			else if (ext == CH_TEXT("png")) {
				for (Lookup<Texture*>& it : loaded_textures) {
					if (ch::streq(it.key.file_name, r.file_name) && it.key.last_write_time < r.last_write_time) {
						ch::File_Data fd;
						if (!load_asset(full_path, &fd)) continue;

						const s32 desired_components = (s32)BT_RGBA;
						stbi_set_flip_vertically_on_load(true);
						Bitmap bm;
						bm.data = stbi_load_from_memory(fd.data, (int)fd.size, &bm.width, &bm.height, &bm.num_components, desired_components);
						defer(stbi_image_free(bm.data));
						if (bm) {
							if (it.value) it.value->free();
							*it.value = Texture(bm);
							o_log(CH_TEXT("reloaded texture %s"), full_path);
						} else {
							o_log_error(CH_TEXT("failed to reload texture %s"), full_path);
						}
						it.key = r;
					}
				}
			}
		}
	}
}

bool load_asset(const ch::Path& path, ch::File_Data* fd) {
	return ch::load_file_into_memory(path, fd, allocator);
}

Shader* find_shader(const tchar* name) {
	for (Lookup<Shader*>& it : loaded_shaders) {
		ch::Path fn = it.key.file_name;
		if (fn.get_filename() == name && it.value) {
			return it.value;
		}
	}

	return get_default_shader();
}

Texture* find_texture(const tchar* name) {
	for (Lookup<Texture*>& it : loaded_textures) {
		ch::Path fn = it.key.file_name;
		if (fn.get_filename() == name && it.value) {
			return it.value;
		}
	}

	return nullptr;
}