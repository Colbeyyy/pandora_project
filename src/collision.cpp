#include "collision.h"
#include "draw.h"

void AABB::debug_draw(const ch::Color& color) {
	draw_border_quad(position, size, 2.f, color);
}

bool AABB::intersects(const AABB& box, AABB* out) const {
	const ch::Vector2 my_min = get_min();
	const ch::Vector2 my_max = get_max();
	const ch::Vector2 r_min = box.get_min();
	const ch::Vector2 r_max = box.get_max();

	if (out) {
		const f32 min_x = (my_min.x < r_min.x) ? r_min.x : my_min.x;
		const f32 min_y = (my_min.y < r_min.y) ? r_min.y : my_min.y;
		const f32 max_x = (my_max.x > r_max.x) ? r_max.x : my_max.x;
		const f32 max_y = (my_max.y > r_max.y) ? r_max.y : my_max.y;


		const ch::Vector2 size(max_x - min_x, max_y - min_y);
		const ch::Vector2 position(min_x + size.x / 2.f, min_y + size.y / 2.f);

		*out = AABB(position, size);
	}

	return !(r_min.x > my_max.x || r_max.x < my_min.x || r_max.y < my_min.y || r_min.y > my_max.y);
}

bool AABB::intersects(ch::Vector2 point) const {
	const ch::Vector2 min = get_min();
	const ch::Vector2 max = get_max();

	return !(point.x < min.x || point.x > max.x || point.y < min.y || point.y > max.y);
}

bool line_intersect(ch::Vector2 a1, ch::Vector2 a2, ch::Vector2 b1, ch::Vector2 b2, ch::Vector2* out_vec) {
	*out_vec = ch::Vector2();

	ch::Vector2 b = a2 - a1;
	ch::Vector2 d = b2 - b1;

	const f32 bd_cross = b.x * d.y - b.y * d.x;

	if (bd_cross == 0.f) return false;

	ch::Vector2 c = b1 - a1;
	const f32 t = (c.x * d.y - c.y * d.x) / bd_cross;
	if (t < 0.f || t > 1.f) return false;

	const f32 u = (c.x * b.y - c.y * b.x) / bd_cross;
	if (u < 0.f || u > 1.f) return false;

	// @NOTE(CHall): Add scalar multiplication
	*out_vec = a1 + ch::Vector2(t) * b;

	return true;
}

bool line_trace_to_aabb(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const AABB& box) {

	const ch::Vector2 min = box.get_min();
	const ch::Vector2 max = box.get_max();

	ch::Vector2 hit_pos;

	auto get_hit_result = [&](ch::Vector2 a, ch::Vector2 b) -> Hit_Result {
		Hit_Result result;
		result.impact = hit_pos;
		const ch::Vector2 hit_angle = (b - a).get_normalized();
		result.normal = ch::Vector2(hit_angle.y, -hit_angle.x);
		return result;
	};

	const ch::Vector2 dir = (end - start).get_normalized();
	const f32 dir_up = ch::Vector2(0.f, 1.f).dot(dir);
	const f32 dir_right = ch::Vector2(1.f, 0.f).dot(dir);

	if (dir_right > 0.5f)
	{
		const ch::Vector2 b1 = min;
		const ch::Vector2 b2 = ch::Vector2(min.x, max.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result(b2, b1);
			return true;
		}
	}

	if (dir_right < 0.5f)
	{
		const ch::Vector2 b1 = max;
		const ch::Vector2 b2 = ch::Vector2(max.x, min.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result(b2, b1);
			return true;
		}
	}

	if (dir_up > 0.5f)
	{
		const ch::Vector2 b1 = min;
		const ch::Vector2 b2 = ch::Vector2(max.x, min.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result(b1, b2);
			return true;
		}
	}

	if (dir_up < 0.5f)
	{
		const ch::Vector2 b1 = max;
		const ch::Vector2 b2 = ch::Vector2(min.x, max.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result(b1, b2);
			return true;
		}
	}

	return false;
}

bool aabb_sweep_to_aabb(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, ch::Vector2 size, AABB box) {
	box.size += size;

	if (line_trace_to_aabb(out_result, start, end, box)) {
		return true;
	}

	*out_result = Hit_Result();

	auto Sign = [](f32 num) -> f32 {
		return (num < 0.f ? -1.f : 1.f);
	};

	box.size -= size;
	AABB out_bb;
	if (AABB(end, size).intersects(box, &out_bb)) {
		if (out_bb.size.x > out_bb.size.y) {
			f32 flip = Sign(end.y - box.position.y);
			end.y += out_bb.size.y * flip;
			out_result->normal.y = flip;
		} else {
			f32 flip = Sign(end.x - box.position.x);
			end.x += out_bb.size.x * flip;
			out_result->normal.x = flip;
		}

		out_result->impact = end;
		return true;
	}

	return false;

}
