#pragma once

#include <ch_stl/math.h>
#include <ch_stl/hash_table.h>

// #include "tile.h"

#include "entity.h"

#include "collision.h"

struct Trace_Details {
	ch::Array<Entity_Id> e_to_ignore;

};

struct World {
	ch::Hash_Table<Entity_Id, Entity*> entities;
	
	ch::Allocator entity_allocator;

	// Tile_Grid tile_grid;

	Entity_Id cam_id;

	World();

	Entity* spawn_entity();
	Entity* find_entity(Entity_Id id);
	bool free_entity(Entity_Id id);

	void tick(f32 dt);

	ch::Vector2 screen_space_to_world_space(ch::Vector2 pos);
	ch::Vector2 world_space_to_screen_space(ch::Vector2 pos);

	bool line_trace(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const Trace_Details& trace_details);
	bool aabb_sweep(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details);
	bool aabb_multi_sweep(ch::Array<Hit_Result>* out_results, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details);
};

World* get_world();