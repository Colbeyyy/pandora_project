#pragma once

#include <ch_stl/math.h>
#include <ch_stl/hash_table.h>

#include "entity.h"
#include "component.h"
#include "system.h"

struct Trace_Details {
	ch::Array<Entity_Id> e_to_ignore;

	Trace_Details() = default;
};

struct World {
	ch::Array<System*> systems;
	ch::Hash_Table<Entity_Id, Entity> entities;
	ch::Array<Component*> components;

	World();

	Entity* spawn_entity();
	Entity* find_entity(Entity_Id id);

	void tick(f32 dt);
	void draw();
};

World* get_world();