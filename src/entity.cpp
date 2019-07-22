#include "entity.h"
#include "draw.h"
#include "ch_stl/time.h"
#include "game_state.h"

void Entity::draw() {
#if BUILD_DEBUG
	// get_bounds().debug_draw();
	const ch::Vector2 size = 10.f;
	draw_border_quad(position.xy, size, 2.f, ch::green);
#endif
}

void Camera::tick(f32 dt) {
	Player* player = get_world()->find_entity<Player>(g_game_state.player_id);

	const f32 tx = player->position.x;
	const f32 ty = player->position.y;

	position.x = ch::interp_to(position.x, tx, dt, 5.f);
	position.y = ch::interp_to(position.y, ty, dt, 1.f);
}

void Camera::draw() {
	// Super::draw();

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

	position.y += ch::sin((f32)current_time) * 1.f * dt;
}

void Block::draw() {
	const ch::Color color = ch::white;
	draw_quad(position.xy, size, color);
	Super::draw();
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


	draw_quad(position.xy, size, 0xFFC0CBFF);
	 draw_line(position.xy, position.xy + velocity.get_normalized() * 100.f, 5.f, ch::green);
	Super::draw();
}

void Player::collision_tick(f32 dt) {
	ch::Vector2 new_position = position.xy + velocity * dt;

	Trace_Details td;
	td.e_to_ignore.push(id);
	td.e_to_ignore.push(get_world()->current_camera->id);
	defer(td.e_to_ignore.destroy());

	const ch::Vector2 start = position.xy;
	const ch::Vector2 end = new_position;

	Hit_Result result;
	if (get_world()->aabb_sweep(&result, start, end, size, td)) {
		const ch::Vector2 d = result.impact - end;
		new_position += result.normal * d.length();
 		velocity -= result.normal * velocity;

		const ch::Vector2 up(0.f, 1.f);
		const f32 dot_up = up.dot(result.normal);

		if (ch::abs(dot_up) > 0.7f) {
			on_ground = true;
			num_jumps = 0;
		}
	}

	position.xy = new_position;
}

bool World::destroy_entity(Entity_Id id) {
	for (usize i = 0; i < entities.count; i++) {
		Entity* e = entities[i];

		if (e && e->id == id) {
			e->destroy();
		}
		return true;
	}

	return false;
}

void World::destroy_all() {
	for (Entity* e : entities) {
		if (e) ch_delete e;
	}

	entities.count = 0;
}

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
			}
			else {
				closest_result = result;
			}
		}
	}

	*out_result = closest_result;

	return closest_result.entity;
}

bool World::aabb_sweep(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, const Trace_Details& trace_details) {

	Hit_Result closest_result = {};

	for (Entity* e : entities) {
		Hit_Result result;
		if (e && !trace_details.e_to_ignore.contains(e->id) && aabb_sweep_to_aabb(&result, start, end, size, e->get_bounds())) {
			result.entity = e;

			if (closest_result.entity) {
				const f32 r_distance = (result.impact - start).length_squared();
				const f32 cr_distance = (closest_result.impact - start).length_squared();

				if (r_distance > cr_distance) {
					closest_result = result;
				}
			}
			else {
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

CH_FORCEINLINE World* get_world() {
	return g_game_state.loaded_world;
}
