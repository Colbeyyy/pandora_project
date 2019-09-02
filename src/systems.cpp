#include "systems.h"
#include "draw.h"
#include "world.h"
#include "components.h"
#include "input.h"
#include "console.h"

void Physics_System::tick(f32 dt) {
	for (Physics_Component* it : Component_Iterator<Physics_Component>(get_world())) {
		if (!it->simulate_physics) continue;

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
		if (get_world()->aabb_multi_sweep(&results, start, end, cc->get_collider().size, td)) {
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

void Player_Movement_System::tick(f32 dt) {
	for (Player_Movement_Component* it : Component_Iterator<Player_Movement_Component>(get_world())) {
		Physics_Component* pc = it->get_sibling<Physics_Component>();
		Transform_Component* tc = it->get_sibling<Transform_Component>();
		Collider_Component* cc = it->get_sibling<Collider_Component>();
		Sprite_Component* sc = it->get_sibling<Sprite_Component>();

		pc->velocity.x = 0.f;

		if (it->on_wall && pc->velocity.y < 0.f) {
			pc->velocity.y = -16.f * 2.f;
			it->num_jumps = 0;
		} else {
			pc->velocity.y -= 16.f * 9.8f * dt;
		}

		if (it->can_jump() && was_key_pressed(CH_KEY_SPACE)) {
			if (pc->velocity.y > 0) {
				pc->velocity.y += it->jump_y_velocity;
			} else {
				pc->velocity.y = it->jump_y_velocity;
			}
			it->num_jumps += 1;
		}

		const bool is_sprinting = is_key_down(CH_KEY_SHIFT);

		const f32 speed = is_sprinting ? it->sprint_speed : it->walk_speed;
		if (is_key_down(CH_KEY_A)) {
			pc->velocity.x = -speed;
		}

		if (is_key_down(CH_KEY_D)) {
			pc->velocity.x = speed;
		}

		Trace_Details td;
		td.e_to_ignore.push(it->owner_id);

		ch::Vector2 new_position = tc->position + pc->velocity * dt;
		ch::Vector2 new_velocity = pc->velocity;

		if (pc->velocity.x < 0.f) {
			sc->flip_horz = true;
		}

		if (pc->velocity.x > 0.f) {
			sc->flip_horz = false;
		}

		const ch::Vector2 start = tc->position;
		const ch::Vector2 end = start + pc->velocity * dt;

		const bool was_on_wall = it->on_wall;

		it->on_wall = false;
		it->on_ground = false;

		ch::Array<Hit_Result> results;
		if (get_world()->aabb_multi_sweep(&results, start, end, cc->get_collider().size, td)) {
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

				if (dot_up > 0.7f) {
					it->on_ground = true;
					it->num_jumps = 0;
				}

				if (ch::abs(dot_right) > 0.7f && pc->velocity.x != 0.f) {
					it->on_wall = true;
				}

				Entity* hit_entity = get_world()->find_entity(result.entity);
				if (hit_entity) {
					Jump_Pad_Component* jpc = hit_entity->find_component<Jump_Pad_Component>();
					if (jpc && dot_up > 0.7f) {
						new_velocity.y = jpc->jump_y_velocity;
					}
				}
			}
			new_position += best_pos;
		}

		tc->position = new_position;
		pc->velocity = new_velocity;
	}
}