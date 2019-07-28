#pragma once

#include <ch_stl/allocator.h>
#include <ch_stl/filesystem.h>

// @NOTE(CHall): Make an actual asset manager
struct Asset_Manager {
	ch::Allocator allocator;

	Asset_Manager() = default;
	explicit Asset_Manager(usize amount);

	bool load_asset(const tchar* path, ch::File_Data* fd);

	CH_FORCEINLINE usize get_current_size() const { return allocator.get_header<ch::Arena_Allocator_Header>()->current;}
};