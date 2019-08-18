#pragma once

#include <ch_stl/math.h>
#include <ch_stl/hash_table.h>

#include "entity.h"
#include "tile.h"

struct World {
	ch::Hash_Table<Entity_Id, Entity*> entities;
	ch::Allocator entity_allocator = ch::get_heap_allocator();

	Camera* current_camera = nullptr;

	Tile_Grid tile_grid;

	World();

	template <typename T>
	T* spawn_entity() {
		T* result = ch_new(entity_allocator) T;
		result->id = get_unique_id();
		entities.push(result->id, result);
		return result;
	}

	template <typename T>
	T* find_entity(Entity_Id id) {
		Entity** result = entities.find(id);
		if (result) return (T*)*result;
		return nullptr;
	}

	bool destroy(Entity_Id id);

	void tick(f32 dt);
	void draw();
};

World* get_world();