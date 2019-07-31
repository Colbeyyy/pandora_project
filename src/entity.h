#pragma once

#include <ch_stl/math.h>
#include <ch_stl/array.h>

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

	bool collision_enabled = true;
	bool tick_enabled = true;

	Entity() = default;

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

	f32 orth_size = 32.f;

	Camera();

	virtual void tick(f32 dt) override;
	virtual void draw() override;

	void set_to_current();

	ch::Vector2 get_mouse_position_in_world() const;

};

struct Block : public Entity {
	using Super = Entity;

	Block();

	virtual void draw() override;

};

struct Player : public Entity {
	using Super = Entity;

	ch::Vector2 velocity;
	ch::Vector2 acceleration;

	bool on_ground = false;
	bool on_wall = false;
	u8 num_jumps = 0;
	const u8 max_jumps = 2;
	const f32 jump_y_velocity = 6.f * 16.f;

	f32 walk_speed = 48.f;
	f32 sprint_speed = 6.f * 16.f;

	virtual void tick(f32 dt) override;
	virtual void draw() override;

	CH_FORCEINLINE bool is_falling() const { return velocity.y < 0.f; }

	void collision_tick(f32 dt);
};
