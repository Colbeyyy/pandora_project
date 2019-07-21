#include "entity.h"
#include "draw.h"
#include "ch_stl/time.h"

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
	const f32 speed = 10.f;

	position.x += speed * dt;
}

void Camera::draw() {
	Super::draw();

	render_from_pos(position.xy, 100.f);
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

	if ((current_time - time_created) > 2.0) {
		destroy();
	}
}

void Block::draw() {
	Super::draw();

	const ch::Vector2 size = 50.f;
	const ch::Color color = ch::white;
	// draw_quad(position.xy, size, color);
}

void Entity::draw() {
	#if BUILD_DEBUG
		get_bounds().debug_draw();
	#endif
}
