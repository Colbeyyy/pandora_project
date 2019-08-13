#include "world.h"
#include "game_state.h"
#include "tile_renderer.h"

#include <ch_stl/allocator.h>

World::World() {
	entities = ch::Array<Entity*>(ch::get_heap_allocator());
}

bool World::destroy_entity(Entity_Id id) {
	for (usize i = 0; i < entities.count; i++) {
		Entity* e = entities[i];

		if (e && e->id == id) {
			e->destroy();
		}
		return true;
	}

	return false;
}

void World::destroy_all() {
	for (Entity* e : entities) {
		if (e) ch_delete e;
	}

	entities.count = 0;
}

bool World::line_trace(struct Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const Trace_Details& trace_details) {
	Hit_Result closest_result = {};

	for (Entity* e : entities) {
		Hit_Result result;
		if (e && e->collision_enabled && !trace_details.e_to_ignore.contains(e->id) && line_trace_to_aabb(&result, start, end, e->get_bounds())) {
			result.entity = e;

			if (closest_result.entity) {
				const f32 r_distance = (result.impact - start).length_squared();
				const f32 cr_distance = (closest_result.impact - start).length_squared();

				if (r_distance < cr_distance) {
					closest_result = result;
				}
			}
			else {
				closest_result = result;
			}
		}
	}

	*out_result = closest_result;

	return closest_result.entity;
}

bool World::aabb_sweep(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details) {

	Hit_Result closest_result = {};

	for (Entity* e : entities) {
		Hit_Result result;
		if (e && e->collision_enabled && !trace_details.e_to_ignore.contains(e->id) && aabb_sweep_to_aabb(&result, start, end, size, e->get_bounds())) {
			result.entity = e;

			if (closest_result.entity) {
				const f32 r_distance = (result.impact - start).length_squared();
				const f32 cr_distance = (closest_result.impact - start).length_squared();

				if (r_distance < cr_distance) {
					closest_result = result;
				}
			}
			else {
				closest_result = result;
			}
		}
	}

	*out_result = closest_result;

	return closest_result.entity;
}

bool World::aabb_multi_sweep(ch::Array<Hit_Result>* out_results, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details) {
	for (Entity* e : entities) {
		Hit_Result result;
		if (e && e->collision_enabled && !trace_details.e_to_ignore.contains(e->id) && aabb_sweep_to_aabb(&result, start, end, size, e->get_bounds())) {
			result.entity = e;

			AABB out_bb;
			AABB(result.impact, size).intersects(result.entity->get_bounds(), &out_bb);
			if (!out_bb.size) continue;

			out_results->push(result);
		}
	}

	return out_results->count > 0;
}

void World::tick(f32 dt) {
	for (usize i = 0; i < entities.count; i++) {
		Entity* e = entities[i];

		if (!e || !e->tick_enabled) continue;

		if (e->is_marked_for_destruction()) {
			entities.remove(i);
			ch_delete e;
			i -= 1;
			continue;
		}

		e->tick(dt);
	}
}

void World::draw() {
	if (current_camera) {
		current_camera->draw();
	}

	for (Entity* e : entities) {
		if (e && e != current_camera) {
			e->draw();
		}
	}

	tile_renderer.flush();

	const ch::Vector2 mouse_pos = current_camera->get_mouse_position_in_world();
	Texture* t = asset_manager.find_texture(CH_TEXT("test_tilesheet"));
	Sprite s;
	s.atlas = t;
	s.width = 16;
	s.height = 16;
	s.x = 1;
	s.y = 0;

	ch::Color color = ch::white;
	color.a = 0.5f;

	draw_sprite(mouse_pos, 16.f, color, s);
}

World* get_world() {
	return game_state.loaded_world;
}