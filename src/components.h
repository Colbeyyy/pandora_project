#pragma once

#include <ch_stl/math.h>
#include "texture.h"

#include "entity_id.h"

enum Component_Type {
	CT_None,
};

struct Component {
	usize type_id;
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

	static usize type_id_counter;
};

template <typename T>
struct TComponent : public Component {
	CH_FORCEINLINE static usize get_type_id() {
		static usize type_id = type_id_counter += 1;
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
};