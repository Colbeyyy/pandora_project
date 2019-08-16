#pragma once

#include <ch_stl/types.h>
#include <ch_stl/array.h>

#include "component.h"

using Entity_Id = u64;
u64 hash(Entity_Id e_id);
Entity_Id get_unique_id();

enum Entity_Flags {
	EF_MarkedForDestruction = 0x01,
	EF_Transient = 0x02,
};

struct Entity {
	Entity_Id id;
	u32 flags = 0;
	ch::Array<Component*> components;

	Entity();

	CH_FORCEINLINE void destroy() {
		flags |= EF_MarkedForDestruction;
	}

	CH_FORCEINLINE bool is_marked_for_destruction() const { return (flags & EF_MarkedForDestruction) != 0; }
};