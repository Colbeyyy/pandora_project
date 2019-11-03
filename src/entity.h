#pragma once

#include <ch_stl/types.h>
#include <ch_stl/array.h>

#include "entity_id.h"

enum Entity_Flags {
	EF_MarkedForDestruction = 0x01,
	EF_Transient = 0x02,
	EF_NoTick = 0x04,
	EF_NoDraw = 0x08,
};

struct Entity {
	Entity_Id id;
	u32 flags = 0;
	const char* name = nullptr;

	CH_FORCEINLINE void destroy() {
		flags |= EF_MarkedForDestruction;
	}

	CH_FORCEINLINE bool is_marked_for_destruction() const { return (flags & EF_MarkedForDestruction) != 0; }
};