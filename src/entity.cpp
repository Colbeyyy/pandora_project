#include "entity.h"
#include "draw.h"
#include "world.h"

#include "input_state.h"
#include "game_state.h"

#include <ch_stl/hash.h>
#include <ch_stl/time.h>

u64 hash(Entity_Id e_id) {
	return ch::fnv1_hash(&e_id, sizeof(e_id));
}

Entity_Id get_unique_id() {
	f64 current_time = ch::get_time_in_seconds();
	return (Entity_Id)(current_time * 1000000000.0);
}

Entity::Entity() {
}

Camera::Camera() {
	size.x = (f32)back_buffer_width;
	size.y = (f32)back_buffer_height;

	ortho_size = (f32)back_buffer_height / 2.f;

	flags |= EF_NoDraw | EF_NoCollision;
}

void Camera::get_view(ch::Matrix4* out) const {
	*out = ch::translate(-position);
}

void Camera::get_projection(ch::Matrix4* out) const {
	const f32 w = (f32)back_buffer_width;
	const f32 h = (f32)back_buffer_height;

	const f32 aspect_ratio = w / h;

	const f32 f = 10.f;
	const f32 n = 1.f;

	*out = ch::ortho(ortho_size, aspect_ratio, f, n);

}

void Camera::set_current() {
	get_world()->current_camera = this;
}

ch::Vector2 Camera::get_mouse_pos_in_world() const {
	const ch::Vector2 back_buffer_size = get_back_buffer_draw_size();
	const ch::Vector2 viewport_size = game_state.window.get_viewport_size();
	ch::Vector2 mouse_pos = input_state.current_mouse_position;

	const f32 width = back_buffer_size.x;
	const f32 height = back_buffer_size.y;

	const f32 offset_x = (f32)viewport_size.ux - width;
	const f32 offset_y = (f32)viewport_size.uy - height;

	mouse_pos.x -= offset_x / 2.f;
	mouse_pos.y -= offset_y / 2.f;

	const f32 x = (2.f * mouse_pos.x) / width - 1.f;
	const f32 y = 1.f - (2.f * mouse_pos.y) / height;

	ch::Matrix4 cam_projection;
	get_projection(&cam_projection);

	ch::Matrix4 cam_view;
	get_view(&cam_view);

	const ch::Vector4 clip_coords(x, y, -1.f, 1.f);
	ch::Vector4 eye_coords = cam_projection.inverse() * clip_coords;
	eye_coords.z = -1.f;
	eye_coords.w = 0.f;

	const ch::Vector4 ray_world = cam_view.inverse() * eye_coords;
	const ch::Vector2 world = ray_world.xy;

	return position + world;
}

void Camera::tick(f32 dt) {
	const f32 speed = 16.f;
	if (input_state.is_key_down(CH_KEY_W)) {
		position.y += 16 * dt;
	}

	if (input_state.is_key_down(CH_KEY_S)) {
		position.y -= 16 * dt;
	}

	if (input_state.is_key_down(CH_KEY_D)) {
		position.x += 16 * dt;
	}

	if (input_state.is_key_down(CH_KEY_A)) {
		position.x -= 16 * dt;
	}
}