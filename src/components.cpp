#include "components.h"
#include "draw.h"

usize Component::type_id_counter;

ch::Matrix4 Camera_Component::get_projection() const {
	const f32 w = (f32)back_buffer_width;
	const f32 h = (f32)back_buffer_height;

	const f32 aspect_ratio = w / h;

	const f32 f = 10.f;
	const f32 n = 1.f;

	return ch::ortho(ortho_size, aspect_ratio, f, n);
}