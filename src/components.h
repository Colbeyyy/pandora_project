#pragma once

#include <ch_stl/math.h>
#include "texture.h"
#include "collision.h"

#include "entity_id.h"

enum Component_Type {
	CT_None,
};

struct Component {
	u64 type_id;
	Entity_Id owner_id;

	template <typename T>
	T* get_sibling() const {
		Entity* e = get_world()->find_entity(owner_id);
		for (Component* c : e->components) {
			if (c->type_id == T::get_type_id()) {
				return (T*)c;
			}
		}

		return nullptr;
	}

	static u64 type_id_counter;
};

template <typename T>
struct TComponent : public Component {
	CH_FORCEINLINE static u64 get_type_id() {
		static u64 type_id = type_id_counter += 1;
		return type_id;
	}

	TComponent() {
		type_id = get_type_id();
	}
};

struct Transform_Component : TComponent<Transform_Component> {
	ch::Vector2 position = 0.f;
	f32 rotation = 0.f;
};

struct Camera_Component : public TComponent<Camera_Component> {
	f32 ortho_size = 32.f;

	ch::Matrix4 get_projection() const;
};

struct Sprite_Component : public TComponent<Sprite_Component> {
	Sprite sprite;
	ch::Vector2 offset;
	bool flip_horz = false;
};

struct Physics_Component : public TComponent<Physics_Component> {
	ch::Vector2 velocity;
	ch::Vector2 acceleration;
	bool simulate_physics = true;
};

struct Collider_Component : public TComponent<Collider_Component> {
	ch::Vector2 size;
	ch::Vector2 offset;
	bool is_blocking = true;

	AABB get_collider() const;
};

struct Player_Movement_Component : public TComponent<Player_Movement_Component> {
	bool on_wall = false;
	bool on_ground = false;
	u8 num_jumps = 0;
	const u8 max_jumps = 1;
	const f32 walk_speed = 16.f * 2.f;
	const f32 sprint_speed = 16.f * 4.f;
	const f32 jump_y_velocity = 16.f * 4.f;

	CH_FORCEINLINE bool can_jump() const {
		return (on_wall || on_ground) && num_jumps < max_jumps;
	}
};

struct Jump_Pad_Component : public TComponent<Jump_Pad_Component> {
	const f32 jump_y_velocity = 16.f * 7.f;
};