#pragma once

#include <ch_stl/types.h>
#include <ch_stl/array.h>

#include "components.h"
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
	ch::Array<Component*> components;

	Entity() {
		components.allocator = ch::get_heap_allocator();
	}

	CH_FORCEINLINE void destroy() {
		flags |= EF_MarkedForDestruction;
	}

	CH_FORCEINLINE bool is_marked_for_destruction() const { return (flags & EF_MarkedForDestruction) != 0; }

	template <typename T>
	T* add_component() {
		return get_world()->add_component_to_entity<T>(id);
	}

	template <typename T>
	T* find_component() {
		for (Component* c : components) {
			if (c->type_id == T::get_type_id()) {
				return (T*)c;
			}
		}

		return nullptr;
	}
};