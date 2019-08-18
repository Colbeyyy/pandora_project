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

		const ch::Vector2 mouse_pos = current_camera->get_mouse_pos_in_world();
		const ch::Vector2 render_pos = tile_grid.round_to_grid(mouse_pos);

		AABB draw_box(current_camera->position, current_camera->size);

		f32 x = draw_box.get_min().x;
		for (u32 i = 0; i < back_buffer_width; i++) {
			ch::Vector2 f(x, 0.f);
			f = tile_grid.round_to_grid(f);
			f += tile_grid.tile_size / 2.f;

			ch::Vector2 start(f.x, draw_box.get_min().y);
			ch::Vector2 end(f.x, draw_box.get_max().y);
			draw_line(start + 0.5f, end + 0.5f, 1.f, ch::white);

			x += tile_grid.tile_size.x;
		}

		f32 y = draw_box.get_min().y;
		for (u32 i = 0; i < back_buffer_height; i++) {
			ch::Vector2 f(0.f, y);
			f = tile_grid.round_to_grid(f);
			f += tile_grid.tile_size / 2.f;

			ch::Vector2 start(draw_box.get_min().x, f.y);
			ch::Vector2 end(draw_box.get_max().x, f.y);
			draw_line(start - 0.5f, end - 0.5f, 1.f, ch::white);

			y += tile_grid.tile_size.y;
		}
	
		draw_quad(render_pos, tile_grid.tile_size, ch::green);

	}

	using Pair = ch::Hash_Table<Entity_Id, Entity*>::Pair;
	for (Pair& it : entities) {
		if ((it.value->flags & EF_NoDraw) != 0) continue;
		it.value->draw();
	}

}

World* get_world() {
	return game_state.loaded_world;
}