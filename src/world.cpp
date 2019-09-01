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

bool World::line_trace(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const Trace_Details& trace_details) {
	Hit_Result closest = {};

	for (Collider_Component* it : Component_Iterator<Collider_Component>(this)) {
		Hit_Result result;
		result.entity = it->owner_id;
		if (trace_details.e_to_ignore.contains(result.entity)) continue;
		 
		if (line_trace_to_aabb(&result, start, end, it->collider)) {
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
		Hit_Result result;
		result.entity = it->owner_id;
		if (trace_details.e_to_ignore.contains(result.entity)) continue;

		if (aabb_sweep_to_aabb(&result, start, end, size, it->collider)) {
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
		Hit_Result result;
		result.entity = it->owner_id;
		if (trace_details.e_to_ignore.contains(result.entity)) continue;

		if (aabb_sweep_to_aabb(&result, start, end, size, it->collider)) {
			AABB out_bb;
			AABB(result.impact, size).intersects(it->collider, &out_bb);
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
	Entity* result = get_world()->spawn_entity();
	Transform_Component* tc = result->add_component<Transform_Component>();
	Sprite_Component* sc = result->add_component<Sprite_Component>();
	Collider_Component* cc = result->add_component<Collider_Component>();
	Physics_Component* pc = result->add_component<Physics_Component>();

	tc->position = position;
	Texture* t = find_texture(CH_TEXT("character"));
	Sprite s(t, 16, 32, 0, 0);
	sc->sprite = s;
	// sc->offset.y = 16.f;

	cc->collider.size = ch::Vector2(16.f, 32.f);

	return result;
}

Entity* spawn_tile(ch::Vector2 position, u32 tile) {
	Entity* result = get_world()->spawn_entity();
	Transform_Component* tc = result->add_component<Transform_Component>();
	Sprite_Component* sc = result->add_component<Sprite_Component>();
	Collider_Component* cc = result->add_component<Collider_Component>();
	
	tc->position = position;
	Texture* t = find_texture(CH_TEXT("test_tilesheet"));
	Sprite s(t, 16, 16, 0, 0);
	sc->sprite = s;

	cc->collider.size = 16.f;

	return result;
}

Entity* spawn_camera(ch::Vector2 position) {
	Entity* result = get_world()->spawn_entity();
	Transform_Component* tc = result->add_component<Transform_Component>();
	Camera_Component* cc = result->add_component<Camera_Component>();
	
	tc->position = position;
	get_world()->cam_id = result->id;

	return result;
}