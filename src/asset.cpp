#include "asset.h"

Asset_Manager::Asset_Manager(usize amount) {
	allocator = ch::make_arena_allocator(amount);
}

bool Asset_Manager::load_asset(const tchar* path, ch::File_Data* fd) {
	return ch::load_file_into_memory(path, fd, allocator);
}

bool Asset_Manager::set_to_res_path() {
	ch::String exe_path = ch::get_app_path();
	defer(exe_path.free());

	if (!ch::set_current_path(exe_path)) return false;
	if (!ch::set_current_path(CH_TEXT("..\\"))) return false;

	return true;
}
