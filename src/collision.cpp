#include "collision.h"
#include "draw.h"

void AABB::debug_draw(const ch::Color& color) {
	draw_border_quad(position, size, 5.f, color);
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

	auto get_hit_result = [&]() -> Hit_Result {
		Hit_Result result;
		result.impact = hit_pos;
		result.normal = (start - end).get_normalized();
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
			*out_result = get_hit_result();
			return true;
		}
	}

	if (dir_right < 0.5f)
	{
		const ch::Vector2 b1 = max;
		const ch::Vector2 b2 = ch::Vector2(max.x, min.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result();
			return true;
		}
	}

	if (dir_up > 0.5f)
	{
		const ch::Vector2 b1 = min;
		const ch::Vector2 b2 = ch::Vector2(max.x, min.y);
		if (line_intersect(start, end, b1, b2, &hit_pos)) {
			*out_result = get_hit_result();
			return true;
		}
	}

	if (dir_up < 0.5f)
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
