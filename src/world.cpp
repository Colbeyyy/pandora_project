#include "world.h"
#include "game_state.h"
#include "draw.h"

#include "collision.h"

#include <ch_stl/allocator.h>

World::World() {
	entities = ch::Hash_Table<Entity_Id, Entity*>(ch::get_heap_allocator());
}

bool World::destroy(Entity_Id id) {
	Entity** e = entities.find(id);
	if (e) ch_delete(entity_allocator, e);
	return entities.remove(id);
}

void World::tick(f32 dt) {

	ch::Array<Entity_Id> marked_to_destroy;

	using Pair = ch::Hash_Table<Entity_Id, Entity*>::Pair;
	for (Pair& it : entities) {
		if ((it.value->flags & EF_NoTick) != 0) continue;

		if (it.value->is_marked_for_destruction()) marked_to_destroy.push(it.key);

		it.value->tick(dt);
	}

	for (const Entity_Id& it : marked_to_destroy) {
		entities.remove(it);
	}
}

void World::draw() {
	if (current_camera) {
		current_camera->get_view(&view);
		current_camera->get_projection(&projection);
	}

	using Pair = ch::Hash_Table<Entity_Id, Entity*>::Pair;
	for (Pair& it : entities) {
		if ((it.value->flags & EF_NoDraw) != 0) continue;
		it.value->draw();
	}

	for (ch::Hash_Table<ch::Vector2, Tile>::Pair& it : tile_grid.tiles) {
		tile_renderer.push(it.key, 0);
	}
	tile_renderer.flush();
}

World* get_world() {
	return game_state.loaded_world;
}