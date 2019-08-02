#include "asset.h"

Asset_Manager::Asset_Manager(usize amount) {
	allocator = ch::make_arena_allocator(amount);
}

bool Asset_Manager::load_asset(const tchar* path, ch::File_Data* fd) {
	return ch::load_file_into_memory(path, fd, allocator);
}

bool Asset_Manager::set_to_res_path() {
	ch::Path exe_path = ch::get_app_path();

	exe_path.remove_until_directory();
	exe_path.remove_until_directory();
	exe_path.append(CH_TEXT("\\res"));

	if (!ch::set_current_path(exe_path)) return false;

	return true;
}
