#include "world.h"
#include "game_state.h"

#include <ch_stl/allocator.h>

World::World() {
	systems.allocator = ch::get_heap_allocator();
	entities = ch::Hash_Table<Entity_Id, Entity>(ch::get_heap_allocator());
	components.allocator = ch::get_heap_allocator();
}

Entity* World::spawn_entity() {
	Entity e;
	e.id = get_unique_id();
	const usize index = entities.push(e.id, e);
	return &entities.buckets[index].value;
}

void World::tick(f32 dt) {
	
}

void World::draw() {
	render_from_pos(0.f, 100.f);

	// tile_renderer.flush();
}

World* get_world() {
	return game_state.loaded_world;
}