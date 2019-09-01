#pragma once

#include <ch_stl/types.h>

struct System {
	bool is_enabled = true;

	virtual void tick(f32 dt) {}
};

struct Collider_System : System {
	virtual void tick(f32 dt);
};

struct Physics_System : System {
	virtual void tick(f32 dt);
};

struct Movement_System : System {
	virtual void tick(f32 dt);
};

#define ALL_SYSTEMS(macro) macro(Collider_System) \
macro(Physics_System) \
macro(Movement_System)
