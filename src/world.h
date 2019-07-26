#pragma once

#include <ch_stl/math.h>
#include <ch_stl/array.h>
#include "entity.h"

struct Trace_Details {
	ch::Array<Entity_Id> e_to_ignore;

	Trace_Details() = default;
};

struct World {
	ch::Array<Entity*> entities;
	Entity_Id last_id = 0;
	Camera* current_camera;

	World();

	template <typename T>
	T* spawn_entity(const ch::Vector3& position) {
		T* result = ch_new T;
		result->id = last_id;
		last_id += 1;
		result->position = position;
		result->flags = 0;

		result->on_created();

		entities.push(result);

		return (T*)entities.back();
	}

	template <typename T>
	T* find_entity(Entity_Id id) {
		for (Entity* e : entities) {
			if (e && e->id == id) {
				return (T*)e;
			}
		}

		return nullptr;
	}

	bool destroy_entity(Entity_Id id);

	void destroy_all();

	bool line_trace(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const Trace_Details& trace_details = Trace_Details());
	bool aabb_sweep(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details = Trace_Details());
	bool aabb_multi_sweep(ch::Array<Hit_Result>* out_results, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details = Trace_Details());
	void tick(f32 dt);
	void draw();
};

World* get_world();