#pragma once

#include <ch_stl/math.h>
#include <ch_stl/array.h>

#include "game_state.h"
#include "collision.h"

using Entity_Id = u64;

enum Entity_Flags {
	EF_MarkedForDestruction = 0x01,
};

struct Entity {
	Entity_Id id;
	u32 flags = 0;
	ch::Vector3 position;
	ch::Vector2 size;

	virtual void on_created() {}
	virtual void tick(f32 dt) {}
	virtual void draw();

	CH_FORCEINLINE AABB get_bounds() const {
		AABB result;
		result.position = position.xy;
		result.size = size;
		return result;
	}

	CH_FORCEINLINE void destroy() {
		flags |= EF_MarkedForDestruction;
	}

	CH_FORCEINLINE bool is_marked_for_destruction() const { return (flags & EF_MarkedForDestruction) != 0; }
};

struct Camera : public Entity {
	using Super = Entity;

	virtual void tick(f32 dt) override;
	virtual void draw() override;

	void set_to_current();

};

struct Block : public Entity {
	using Super = Entity;

	f64 time_created;

	virtual void on_created() override;
	virtual void tick(f32 dt) override;
	virtual void draw() override;

};

struct World {
	ch::Array<Entity*> entities;
	Entity_Id last_id = 0;
	Camera* current_camera;

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

	bool destroy_entity(Entity_Id id) {
		for (usize i = 0; i < entities.count; i++) {
			Entity* e = entities[i];

			if (e && e->id == id) {
				e->destroy();
			}
			return true;
		}

		return false;
	}

	void tick(f32 dt);
	void draw();
};

CH_FORCEINLINE World* get_world() {
	return g_game_state.loaded_world;
}
