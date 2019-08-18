#pragma once

#include <ch_stl/types.h>

struct System {
	virtual void tick(f32 dt) = 0;
};

struct Camera_System : public System {
	virtual void tick(f32 dt);
};