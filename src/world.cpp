#include "world.h"
#include "game_state.h"
#include "draw.h"
#include "collision.h"

#include <ch_stl/pool_allocator.h>

World::World() {
	entities = ch::Hash_Table<Entity_Id, Entity>(ch::get_heap_allocator());
	components.allocator = ch::get_heap_allocator();
	systems = ch::Hash_Table<const tchar*, System*>(ch::get_heap_allocator());

	component_allocator = ch::make_pool_allocator(512, 1024 * 256);

	usize system_size = 0;
#define SYSTEMS_SIZE(name) system_size += sizeof(name);
	ALL_SYSTEMS(SYSTEMS_SIZE);
#undef SYSTEMS_SIZE
	assert(system_size);

	system_allocator = ch::make_arena_allocator(system_size);
#define PUSH_SYSTEM(name) systems.push(#name, ch_new(system_allocator) name);
	ALL_SYSTEMS(PUSH_SYSTEM);
#undef PUSH_SYSTEM	
}

Entity* World::spawn_entity() {
	Entity e;
	e.id = get_unique_id();
	const usize i = entities.push(e.id, e);
	return &entities.buckets[i].value;
}

Entity* World::find_entity(Entity_Id id) {
	return entities.find(id);
}

bool World::free_entity(Entity_Id id) {
	return entities.remove(id);
}

void World::tick(f32 dt) {
	for (ch::Hash_Table<const tchar*, System*>::Pair& it : systems) {
		if (it.value->is_enabled) {
			it.value->tick(dt);
		}
	}
}

void World::draw() {
	Entity* e = find_entity(cam_id);
	Camera_Component* cc = e->find_component<Camera_Component>();
	Transform_Component* tc = e->find_component<Transform_Component>();

	view = ch::translate(-tc->position);
	projection = cc->get_projection();
}

World* get_world() {
	return game_state.loaded_world;
}
