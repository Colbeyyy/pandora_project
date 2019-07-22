#include "entity.h"
#include "draw.h"
#include "ch_stl/time.h"
#include "game_state.h"


bool World::line_trace(struct Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const Trace_Details& trace_details) {
	Hit_Result closest_result = {};

	for (Entity* e : entities) {
		Hit_Result result;
		if (e && !trace_details.e_to_ignore.contains(e->id) && line_trace_to_aabb(&result, start, end, e->get_bounds())) {
			result.entity = e;

			if (closest_result.entity) {
				const f32 r_distance = (result.impact - start).length_squared();
				const f32 cr_distance = (closest_result.impact - start).length_squared();
				
				if (r_distance > cr_distance) {
					closest_result = result;
				}
			} else {
				closest_result = result;
			}
		}
	}

	*out_result = closest_result;

	return closest_result.entity;
}

void World::tick(f32 dt) {
	for (usize i = 0; i < entities.count; i++) {
		Entity* e = entities[i];
		
		if (!e) continue;

		if (e->is_marked_for_destruction()) {
			entities.remove(i);
			ch_delete e;
			i -= 1;
			continue;
		}

		e->tick(dt);
	}
}

void World::draw() {
	if (current_camera) {
		current_camera->draw();
	}

	for (Entity* e : entities) {
		if (e && e != current_camera) {
			e->draw();
		}
	}
}

void Camera::tick(f32 dt) {
	Player* player = get_world()->find_entity<Player>(g_game_state.player_id);

	const f32 tx = player->position.x;
	const f32 ty = player->position.y;

	position.x = ch::interp_to(position.x, tx, dt, 5.f);
	position.y = ch::interp_to(position.y, ty, dt, 1.f);
}

void Camera::draw() {
	Super::draw();

	render_from_pos(position.xy, 512.f);
}

void Camera::set_to_current() {
	World* world = get_world();

	if (world) {
		world->current_camera = this;
	}
}

void Block::on_created() {
	time_created = ch::get_ms_time();
}

void Block::tick(f32 dt) {
	const f64 current_time = ch::get_ms_time();
}

void Block::draw() {

	const ch::Color color = ch::white;
	draw_quad(position.xy, size, color);
	Super::draw();
}

void Entity::draw() {
	#if BUILD_DEBUG
		// get_bounds().debug_draw();
	#endif
}

void Player::tick(f32 dt) {

	velocity.y = ch::max(velocity.y, -(980.f * 980.f));

	velocity.x = 0.f;
	velocity.y -= 980.f * dt;

	if (num_jumps < max_jumps &&  g_input_state.was_key_pressed(' ')) {
		velocity.y = jump_y_velocity;
		num_jumps += 1;
	}

	const bool is_sprinting = g_input_state.is_key_down(16);

	f32 speed = is_sprinting ? sprint_speed : walk_speed;

	if (g_input_state.is_key_down('A')) {
		velocity.x = -speed;
	}

	if (g_input_state.is_key_down('D')) {
		velocity.x = speed;
	}

	collision_tick(dt);
}

void Player::draw() {

	size = ch::Vector2(40.f, 100.f);

	draw_quad(position.xy, size, ch::cyan);
	Super::draw();
}

void Player::collision_tick(f32 dt) {
	position.xy += velocity * dt;

	World* world = get_world();
	if (world) {
		const AABB my_bb = get_bounds();
		for (Entity* e : world->entities) {
			if (e && e != world->current_camera && e != this) {
				AABB out_bb;
				if (my_bb.intersects(e->get_bounds(), &out_bb)) {
					if (out_bb.size.x > out_bb.size.y) {
						f32 flip = 1.f;
						if (position.y < e->position.y) flip = -1.f;
						position.y += out_bb.size.y * flip;

						if (flip == 1.f) {
							num_jumps = 0;
							on_ground = true;
						}

						velocity.y = 0.f;
					} else {
						f32 flip = 1.f;
						if (position.x < e->position.x) flip = -1.f;
						position.x += out_bb.size.x * flip;
						velocity.x = 0.f;
					}
				}
			}
		}
	}
}

CH_FORCEINLINE World* get_world() {
	return g_game_state.loaded_world;
}
