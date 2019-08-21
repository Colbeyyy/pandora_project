#include "systems.h"
#include "draw.h"
#include "world.h"
#include "components.h"
#include "input.h"

void Camera_System::tick(f32 dt) {
	World* world = get_world();

	ch::Vector2 dir = 0.f;

	if (is_key_down(CH_KEY_W)) {
		dir.y = 1;
	}

	if (is_key_down(CH_KEY_S)) {
		dir.y = -1;
	}

	if (is_key_down(CH_KEY_D)) {
		dir.x = 1;
	}

	if (is_key_down(CH_KEY_A)) {
		dir.x = -1;
	}

	for (Camera_Component* it : Component_Iterator<Camera_Component>(world)) {
		it->ortho_size = (f32)back_buffer_height / 2.f;
		projection = it->get_projection();
		Transform_Component* t = it->get_sibling<Transform_Component>();

		t->position += dir * 16.f * dt;
		view = ch::translate(-t->position);
	}
}

void Sprite_System::tick(f32 dt) {

}
