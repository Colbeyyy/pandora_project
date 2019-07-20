#pragma once

#include <ch_stl/math.h>
#include <ch_stl/array.h>

using Entity_Id = u64;

struct Entity;

struct World {
	ch::Array<Entity> entities;
	Entity_Id last_id = 0;

	template <typename T>
	T* spawn_entity(const ch::Vector3& position) {
		
	}


};

enum Entity_Flags {
	EF_MarkedForDestruction = 0x01,
};

struct Entity {
	Entity_Id id;
	u32 flags;
	ch::Vector3 position;

	virtual void tick(f32 dt) {}
	virtual void render() {}
};

