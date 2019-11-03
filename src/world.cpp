#include "world.h"
#include "game.h"
#include "draw.h"
#include "collision.h"
#include "console.h"

#include <ch_stl/pool_allocator.h>
#include "texture.h"

World::World() {
	entities = ch::Hash_Table<Entity_Id, Entity*>(ch::get_heap_allocator());
}

Entity* World::spawn_entity() {
	return nullptr;
}

Entity* World::find_entity(Entity_Id id) {
	return *entities.find(id);
}

bool World::free_entity(Entity_Id id) {
	return entities.remove(id);
}

void World::tick(f32 dt) {
}

ch::Vector2 World::screen_space_to_world_space(ch::Vector2 pos) {
	Entity* camera = find_entity(cam_id);
	if (!camera) return 0.f;

	return 0.f;

	/*
	const ch::Vector2 back_buffer_size = get_back_buffer_draw_size();
	const ch::Vector2 viewport_size = the_window.get_viewport_size();

	const f32 width = back_buffer_size.x;
	const f32 height = back_buffer_size.y;

	const f32 offset_x = (f32)viewport_size.ux - width;
	const f32 offset_y = (f32)viewport_size.uy - height;

	pos.x -= offset_x / 2.f;
	pos.y -= offset_y / 2.f;

	const f32 x = (2.f * pos.x) / width - 1.f;
	const f32 y = 1.f - (2.f * pos.y) / height;

	Camera_Component* cc = camera->find_component<Camera_Component>();
	Transform_Component* tc = camera->find_component<Transform_Component>();

	ch::Matrix4 cam_projection = cc->get_projection();
	ch::Matrix4 cam_view = ch::translate(-tc->position);

	const ch::Vector4 clip_coords(x, y, -1.f, 1.f);
	ch::Vector4 eye_coords = cam_projection.inverse() * clip_coords;
	eye_coords.z = -1.f;
	eye_coords.w = 0.f;

	const ch::Vector4 ray_world = cam_view.inverse() * eye_coords;
	const ch::Vector2 world = ray_world.xy;

	return tc->position + world;
	*/
}

ch::Vector2 World::world_space_to_screen_space(ch::Vector2 pos) {
	Entity* camera = find_entity(cam_id);
	if (!camera) return 0.f;

	return 0.f;

	/*
	const ch::Vector2 back_buffer_size = get_back_buffer_draw_size();
	const ch::Vector2 viewport_size = the_window.get_viewport_size();

	const f32 width = back_buffer_size.x;
	const f32 height = back_buffer_size.y;

	const f32 offset_x = (f32)viewport_size.ux - width;
	const f32 offset_y = (f32)viewport_size.uy - height;

	pos.x -= offset_x / 2.f;
	pos.y -= offset_y / 2.f;

	const f32 x = (2.f * pos.x) / width - 1.f;
	const f32 y = 1.f - (2.f * pos.y) / height;

	Camera_Component* cc = camera->find_component<Camera_Component>();
	Transform_Component* tc = camera->find_component<Transform_Component>();

	ch::Matrix4 cam_projection = cc->get_projection();
	ch::Matrix4 cam_view = ch::translate(-tc->position);

	const ch::Vector4 clip_coords(x, y, -1.f, 1.f);
	ch::Vector4 eye_coords = cam_projection * clip_coords;
	eye_coords.z = -1.f;
	eye_coords.w = 0.f;

	const ch::Vector4 ray_world = cam_view * eye_coords;
	const ch::Vector2 world = ray_world.xy;

	return tc->position + world;
	*/
}

bool World::line_trace(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const Trace_Details& trace_details) {
	Hit_Result closest = {};

	/*
	for (Collider_Component* it : Component_Iterator<Collider_Component>(this)) {
		if (!it->is_blocking) return false;

		Hit_Result result;
		result.entity = it->owner_id;
		if (trace_details.e_to_ignore.contains(result.entity)) continue;
		 
		if (line_trace_to_aabb(&result, start, end, it->get_collider())) {
			if (closest.entity != 0) {
				const f32 r_distance = (result.impact - start).length_squared();
				const f32 cr_distance = (closest.impact - start).length_squared();

				if (r_distance < cr_distance) {
					closest = result;
				}
			} else {
				closest = result;
			}
		}
	}
	*/

	*out_result = closest;

	return closest.entity != 0;
}

bool World::aabb_sweep(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details) {
	Hit_Result closest = {};

	/*
	for (Collider_Component* it : Component_Iterator<Collider_Component>(this)) {
		if (!it->is_blocking) return false;

		Hit_Result result;
		result.entity = it->owner_id;
		if (trace_details.e_to_ignore.contains(result.entity)) continue;

		if (aabb_sweep_to_aabb(&result, start, end, size, it->get_collider())) {
			if (closest.entity != 0) {
				const f32 r_distance = (result.impact - start).length_squared();
				const f32 cr_distance = (closest.impact - start).length_squared();

				if (r_distance < cr_distance) {
					closest = result;
				}
			} else {
				closest = result;
			}
		}
	}
	*/
	*out_result = closest;

	return closest.entity != 0;
}

bool World::aabb_multi_sweep(ch::Array<Hit_Result>* out_results, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details) {
	/*
	for (Collider_Component* it : Component_Iterator<Collider_Component>(this)) {
		if (!it->is_blocking) return false;

		Hit_Result result;
		if (trace_details.e_to_ignore.contains(it->owner_id)) continue;

		if (aabb_sweep_to_aabb(&result, start, end, size, it->get_collider())) {
			result.entity = it->owner_id;

			AABB out_bb;
			AABB(result.impact, size).intersects(it->get_collider(), &out_bb);
			if (!out_bb.size) continue;

			out_results->push(result);
		}
	}

	*/
	return out_results->count > 0;
}

World* get_world() {
	return loaded_world;
}