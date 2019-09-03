#include "world.h"
#include "game.h"
#include "draw.h"
#include "collision.h"
#include "console.h"

#include <ch_stl/pool_allocator.h>
#include "texture.h"

World::World() {
	entities = ch::Hash_Table<Entity_Id, Entity>(ch::get_heap_allocator());
	components.allocator = ch::get_heap_allocator();
	systems = ch::Hash_Table<const tchar*, System*>(ch::get_heap_allocator());

	component_allocator = ch::make_pool_allocator(512, 1024 * 256);

	usize system_size = 0;
#define SYSTEMS_SIZE(name) system_size += sizeof(name);
	ALL_SYSTEMS(SYSTEMS_SIZE);
#undef SYSTEMS_SIZE
	
	if (!system_size) {
		o_log_error(CH_TEXT("There are no systems defined in systems.h"));
		return;
	}

	system_allocator = ch::make_arena_allocator(system_size);
#define PUSH_SYSTEM(name) systems.push(#name, ch_new(system_allocator) name);
	ALL_SYSTEMS(PUSH_SYSTEM);
#undef PUSH_SYSTEM	
}

Entity* World::spawn_entity(const tchar* name) {
	Entity e;
	e.name = name;
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

ch::Vector2 World::screen_space_to_world_space(ch::Vector2 pos) {
	Entity* camera = find_entity(cam_id);
	if (!camera) return 0.f;

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
}

ch::Vector2 World::world_space_to_screen_space(ch::Vector2 pos) {
	Entity* camera = find_entity(cam_id);
	if (!camera) return 0.f;

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
}

bool World::line_trace(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const Trace_Details& trace_details) {
	Hit_Result closest = {};

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

	*out_result = closest;

	return closest.entity != 0;
}

bool World::aabb_sweep(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details) {
	Hit_Result closest = {};

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

	*out_result = closest;

	return closest.entity != 0;
}

bool World::aabb_multi_sweep(ch::Array<Hit_Result>* out_results, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details) {
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

	return out_results->count > 0;
}

World* get_world() {
	return loaded_world;
}

Entity* spawn_player(ch::Vector2 position) {
	Entity* result = get_world()->spawn_entity(CH_TEXT("player"));
	Transform_Component* tc = result->add_component<Transform_Component>();
	Sprite_Component* sc = result->add_component<Sprite_Component>();
	Collider_Component* cc = result->add_component<Collider_Component>();
	Player_Movement_Component* pmc = result->add_component<Player_Movement_Component>();
	Physics_Component* pc = result->add_component<Physics_Component>();

	pc->simulate_physics = false;
	tc->position = position;
	Texture* t = find_texture(CH_TEXT("character"));
	Sprite s(t, 16, 32, 0, 0);
	sc->sprite = s;

	cc->size = ch::Vector2(14.f, 32.f);

	return result;
}

Entity* spawn_tile(ch::Vector2 position, u32 tile) {
	Entity* result = get_world()->spawn_entity(CH_TEXT("tile"));
	Transform_Component* tc = result->add_component<Transform_Component>();
	Sprite_Component* sc = result->add_component<Sprite_Component>();
	Collider_Component* cc = result->add_component<Collider_Component>();
	
	tc->position = position;
	Texture* t = find_texture(CH_TEXT("test_tilesheet"));
	Sprite s(t, 16, 16, 0, 0);
	sc->sprite = s;

	cc->size = 16.f;

	return result;
}

Entity* spawn_camera(ch::Vector2 position) {
	Entity* result = get_world()->spawn_entity(CH_TEXT("camera"));
	Transform_Component* tc = result->add_component<Transform_Component>();
	Camera_Component* cc = result->add_component<Camera_Component>();
	
	tc->position = position;
	get_world()->cam_id = result->id;

	return result;
}

Entity* spawn_jump_pad(ch::Vector2 position) {
	Entity* result = get_world()->spawn_entity(CH_TEXT("jump pad"));
	Transform_Component* tc = result->add_component<Transform_Component>();
	Sprite_Component* sc = result->add_component<Sprite_Component>();
	Collider_Component* cc = result->add_component<Collider_Component>();
	Jump_Pad_Component* jpc = result->add_component<Jump_Pad_Component>();

	tc->position = position;
	cc->size = ch::Vector2(16.f, 8.f);
	cc->offset.y = -5.f;

	Texture* t = find_texture(CH_TEXT("test_tilesheet"));
	Sprite s(t, 16, 16, 2, 0);
	sc->sprite = s;

	return result;
}
