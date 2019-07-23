#include "entity.h"
#include "draw.h"
#include "game_state.h"
#include "world.h"
#include "ch_stl/time.h"

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
}

void Block::draw() {
	const ch::Color color = ch::white;
	draw_quad(position.xy, size, color);
	Super::draw();
}

void Player::tick(f32 dt) {
	size = ch::Vector2(40.f, 100.f);

	// velocity.y = ch::max(velocity.y, -(980.f * 980.f));

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
	draw_quad(position.xy, size, on_ground ? 0xFFC0CBFF : ch::red);
	draw_line(position.xy, position.xy + velocity * 0.5f, 5.f, ch::green);

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

	on_ground = false;

	ch::Array<Hit_Result> results;
	defer(results.destroy());
	if (get_world()->aabb_multi_sweep(&results, start, end, size, td)) {
		for (const Hit_Result& result : results) {
			const ch::Vector2 d = ch::abs(result.impact - end);
			new_position += result.normal * d;
			velocity -= result.normal * velocity;

			const ch::Vector2 up(0.f, 1.f);
			const f32 dot_up = up.dot(result.normal);

			if (ch::abs(dot_up) > 0.7f) {
				on_ground = true;
				num_jumps = 0;
			}
		}
	}

	position.xy = new_position;
}
