#pragma once

#include <ch_stl/math.h>

struct AABB {

	void debug_draw(const ch::Color& color = ch::green);

	ch::Vector2 position;
	ch::Vector2 size;

	CH_FORCEINLINE ch::Vector2 get_min() const {
		return position - (size / 2.f);
	}

	CH_FORCEINLINE ch::Vector2 get_max() const {
		return position + (size / 2.f);
	}
};

struct Hit_Result {
	ch::Vector2 impact;
	ch::Vector2 normal;
};

bool line_intersect(ch::Vector2 a1, ch::Vector2 a2, ch::Vector2 b1, ch::Vector2 b2, ch::Vector2* out_vec);

bool line_trace(Hit_Result* out_result, ch::Vector2 start, ch::Vector2 end, const AABB& box);