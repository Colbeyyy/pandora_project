#pragma once

#include <ch_stl/math.h>
#include <ch_stl/hash_table.h>

#include "entity.h"
#include "components.h"
#include "systems.h"

struct Trace_Details {
	ch::Array<Entity_Id> e_to_ignore;

	Trace_Details() = default;
};

struct World {
	ch::Array<System> systems;
	ch::Hash_Table<Entity_Id, Entity> entities;
	ch::Array<Component> components;
	ch::Allocator component_allocator;

	World();

	Entity* spawn_entity();

	CH_FORCEINLINE Entity* find_entity(Entity_Id id) {
		return entities.find(id);
	}

	void tick(f32 dt);
	void draw();
};

World* get_world();