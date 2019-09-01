#include "systems.h"
#include "draw.h"
#include "world.h"
#include "components.h"
#include "input.h"
#include "console.h"

void Physics_System::tick(f32 dt) {
	for (Physics_Component* it : Component_Iterator<Physics_Component>(get_world())) {
		Transform_Component* tc = it->get_sibling<Transform_Component>();
		Collider_Component* cc = it->get_sibling<Collider_Component>();

		it->velocity.y -= 16.f * 9.8f * dt;

		Trace_Details td;
		td.e_to_ignore.push(it->owner_id);

		ch::Vector2 new_position = tc->position + it->velocity * dt;
		ch::Vector2 new_velocity = it->velocity;

		const ch::Vector2 start = tc->position;
		const ch::Vector2 end = start + it->velocity * dt;

		ch::Array<Hit_Result> results;
		if (get_world()->aabb_multi_sweep(&results, start, end, cc->collider.size, td)) {
			ch::Vector2 best_pos = 0.f;
			for (Hit_Result& result : results) {
				const ch::Vector2 d = ch::abs(result.impact - end);

				const ch::Vector2 possible_pos = result.normal * d;

				if (ch::abs(best_pos.x) < ch::abs(possible_pos.x)) best_pos.x = possible_pos.x;
				if (ch::abs(best_pos.y) < ch::abs(possible_pos.y)) best_pos.y = possible_pos.y;

				const ch::Vector2 up(0.f, 1.f);
				const ch::Vector2 right(1.f, 0.f);
				const f32 dot_up = up.dot(result.normal);
				const f32 dot_right = right.dot(result.normal);

				new_velocity -= ch::abs(result.normal) * new_velocity;
			}
			new_position += best_pos;
		}

		tc->position = new_position;
		it->velocity = new_velocity;
	}
}

void Movement_System::tick(f32 dt) {
	const f32 speed = 16.f * 3.f;
	for (Physics_Component* it : Component_Iterator<Physics_Component>(get_world())) {
		it->velocity.x = 0.f;

		if (is_key_down(CH_KEY_D)) it->velocity.x = speed;
		if (is_key_down(CH_KEY_A)) it->velocity.x = -speed;

		if (was_key_pressed(CH_KEY_SPACE)) {
			it->velocity.y = 0.f;

			it->velocity.y = 16.f * 6.f;
		}

		Sprite_Component* sc = it->get_sibling<Sprite_Component>();
		if (it->velocity.x > 0.f) {
			sc->flip_horz = false;
		} else if (it->velocity.x < 0.f) {
			sc->flip_horz = true;
		}
	}
}

void Collider_System::tick(f32 dt) {
	for (Collider_Component* it : Component_Iterator<Collider_Component>(get_world())) {
		Transform_Component* tc = it->get_sibling<Transform_Component>();
		it->collider.position = tc->position;
	}
}
