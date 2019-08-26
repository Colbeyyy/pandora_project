#include "systems.h"
#include "draw.h"
#include "world.h"
#include "components.h"
#include "input.h"

void Camera_System::tick(f32 dt) {
	const f32 speed = 16.f * 2.f;
	for (Camera_Component* it : Component_Iterator<Camera_Component>(get_world())) {
		Transform_Component* tc = it->get_sibling<Transform_Component>();
		tc->position.x += speed * dt;
	}
}
