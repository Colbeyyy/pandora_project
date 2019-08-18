#pragma once

#include <ch_stl/math.h>

using Entity_Id = u64;
u64 hash(Entity_Id e_id);
Entity_Id get_unique_id();

enum Entity_Flags {
	EF_MarkedForDestruction = 0x01,
	EF_Transient = 0x02,
	EF_NoTick = 0x04,
	EF_NoDraw = 0x08,
	EF_NoCollision = 0x10,
};

struct Entity {
	Entity_Id id;
	u32 flags = 0;

	ch::Vector2 position;
	ch::Vector2 size;

	Entity();

	virtual void tick(f32 dt) {}
	virtual void draw() {}

	CH_FORCEINLINE void destroy() {
		flags |= EF_MarkedForDestruction;
	}

	CH_FORCEINLINE bool is_marked_for_destruction() const { return (flags & EF_MarkedForDestruction) != 0; }
};

struct Camera : public Entity {
	using Super = Entity;

	f32 ortho_size = 32.f;

	Camera();

	void get_view(ch::Matrix4* out) const;
	void get_projection(ch::Matrix4* out) const;

	void set_current();

	ch::Vector2 get_mouse_pos_in_world() const;

	virtual void tick(f32 dt) override;
};