#pragma once

#include <ch_stl/math.h>

struct AABB {

	ch::Vector2 position;
	ch::Vector2 size;

	AABB() = default;
	AABB(ch::Vector2 _pos, ch::Vector2 _size) : position(_pos), size(_size) {}

	void debug_draw(const ch::Color& color = ch::green);

	CH_FORCEINLINE ch::Vector2 get_min() const {
		return position - (size / 2.f);
	}

	CH_FORCEINLINE ch::Vector2 get_max() const {
		return position + (size / 2.f);
	}

	bool intersects(const AABB& box, AABB* out = nullptr) const;
	bool intersects(ch::Vector2 point) const;
};

struct Entity;

struct Hit_Result {
	ch::Vector2 impact;
	ch::Vector2 normal;
	Entity* entity;
};

bool line_intersect(ch::Vector2 a1, ch::Vector2 a2, ch::Vector2 b1, ch::Vector2 b2, ch::Vector2* out_vec);

bool line_trace_to_aabb(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const AABB& box);