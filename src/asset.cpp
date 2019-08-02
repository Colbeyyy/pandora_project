#include "asset.h"

Asset_Manager::Asset_Manager(usize amount) {
	allocator = ch::make_arena_allocator(amount);
}

bool Asset_Manager::load_asset(const tchar* path, ch::File_Data* fd) {
	return ch::load_file_into_memory(path, fd, allocator);
}

bool Asset_Manager::set_to_res_path() {
	ch::Path exe_path = ch::get_app_path();

	const ssize last_slash = 0;// exe_path.find_from_right('\\');
	if (last_slash < 0) return false;

	exe_path.count = last_slash + 1;
	exe_path.data[exe_path.count] = 0;

	if (!ch::set_current_path(exe_path)) return false;
	if (!ch::set_current_path(CH_TEXT("..\\res"))) return false;

	return true;
}
