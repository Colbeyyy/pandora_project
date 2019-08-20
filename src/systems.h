#pragma once

#include <ch_stl/types.h>

struct System {
	virtual void tick(f32 dt) {}
};