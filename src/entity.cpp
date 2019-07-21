#include "entity.h"
#include "draw.h"
#include "ch_stl/time.h"

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
	size = 100.f;
}

void Block::tick(f32 dt) {
	const f64 current_time = ch::get_ms_time();
}

void Block::draw() {

	const ch::Vector2 size = 100.f;
	const ch::Color color = ch::white;
	draw_quad(position.xy, size, color);
	Super::draw();
}

void Entity::draw() {
	#if BUILD_DEBUG
		get_bounds().debug_draw();
	#endif
}

void Player::tick(f32 dt) {
	Hit_Result hit;
	const ch::Vector2 half_height(0.f, size.y / 2.f);

	const ch::Vector2 start = position.xy - half_height;
	const ch::Vector2 end = start + velocity * dt;

	Trace_Details details;
	details.e_to_ignore.push(id);
	defer(details.e_to_ignore.destroy());
	if (get_world()->line_trace(&hit, start, end, details)) {
		position = hit.impact + half_height;
		velocity.y = 0.f;
	} else {
		position = end + half_height;
		velocity.y -= 980.f * dt;
	}

	if (space_pressed) {
		velocity.y = 400.f;
	}
}

void Player::draw() {

	size = ch::Vector2(20.f, 100.f);

	draw_quad(position.xy, size, ch::cyan);
	Super::draw();
}


