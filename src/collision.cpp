#include "collision.h"
#include "draw.h"

void AABB::debug_draw(const ch::Color& color) {
	draw_quad(position, size, color);
}

bool line_intersect(ch::Vector2 a1, ch::Vector2 a2, ch::Vector2 b1, ch::Vector2 b2, ch::Vector2* out_vec) {
	*out_vec = ch::Vector2();

	ch::Vector2 b = a2 - a1;
	ch::Vector2 d = b2 - b1;

	const f32 bd_dot = b.dot(d);

	if (bd_dot == 0.f) return false;

	ch::Vector2 c = b1 - a1;
	const f32 t = c.dot(d) / bd_dot;
	if (t < 0.f || t > 1.f) return false;

	const f32 u = c.dot(b) / bd_dot;
	if (u < 0.f || u > 1.f) return false;

	// @NOTE(CHall): Add scalar multiplication
	*out_vec = a1 + ch::Vector2(t) * b;

	return true;
}

bool line_trace(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const AABB& box) {

	const ch::Vector2 min = box.get_min();
	const ch::Vector2 max = box.get_max();

	ch::Vector2 hit_pos;

	auto get_hit_result = [&]() -> Hit_Result {
		Hit_Result result;
		result.impact = hit_pos;
		result.normal = (end - start).get_normalized();
		return result;
	};

	{
		const ch::Vector2 b1 = min;
		const ch::Vector2 b2 = ch::Vector2(min.x, max.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result();
			return true;
		}
	}

	{
		const ch::Vector2 b1 = max;
		const ch::Vector2 b2 = ch::Vector2(max.x, min.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result();
			return true;
		}
	}

	{
		const ch::Vector2 b1 = min;
		const ch::Vector2 b2 = ch::Vector2(max.x, min.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result();
			return true;
		}
	}

	{
		const ch::Vector2 b1 = max;
		const ch::Vector2 b2 = ch::Vector2(min.x, max.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result();
			return true;
		}
	}

	return false;
}
