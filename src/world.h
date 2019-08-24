#pragma once

#include <ch_stl/math.h>
#include <ch_stl/hash_table.h>

// #include "tile.h"

#include "entity.h"
#include "components.h"
#include "systems.h"

struct World {
	ch::Hash_Table<Entity_Id, Entity> entities;
	ch::Array<Component*> components;
	ch::Hash_Table<const tchar*, System*> systems;
	
	ch::Allocator component_allocator;
	ch::Allocator system_allocator;

	// Tile_Grid tile_grid;

	Entity_Id cam_id;

	World();

	Entity* spawn_entity();
	Entity* find_entity(Entity_Id id);
	bool free_entity(Entity_Id id);

	template <typename T>
	T* add_component_to_entity(Entity_Id e_id) {
		Entity* e = find_entity(e_id);
		if (!e) return nullptr;
		T* c = ch_new(component_allocator) T;
		c->owner_id = e_id;
		e->components.push(c);
		components.push(c);
		return c;
	}

	void tick(f32 dt);
	void draw();
};

World* get_world();

template <typename T>
struct Component_Iterator {
	ch::Array<T*> components;

	Component_Iterator(World* world) {
		for (Component* c : world->components) {
			if (c->type_id == T::get_type_id()) {
				components.push((T*)c);
			}
		}
	}

	~Component_Iterator() {
		components.free();
	}

	T** begin() {
		return components.begin();
	}

	T** end() {
		return components.end();
	}

	const T** cbegin() const {
		return components.cbegin();
	}

	const T** cend() const {
		return components.cend();
	}
};