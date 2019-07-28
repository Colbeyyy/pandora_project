#include "asset.h"

Asset_Manager::Asset_Manager(usize amount) {
	allocator = ch::make_arena_allocator(amount);
}

bool Asset_Manager::load_asset(const tchar* path, ch::File_Data* fd) {
	return ch::load_file_into_memory(path, fd, allocator);
}
