#pragma once

#include <ch_stl/types.h>

struct System {
	bool is_enabled = true;

	virtual void tick(f32 dt) {}
};

struct Physics_System : System {
	virtual void tick(f32 dt);
};

struct Player_Movement_System : System {
	virtual void tick(f32 dt);
};

#define ALL_SYSTEMS(macro) \
macro(Physics_System) \
macro(Player_Movement_System)
