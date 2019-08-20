#pragma once

#include <ch_stl/types.h>

struct System {
	bool is_enabled = true;

	virtual void tick(f32 dt) {}
};

struct Camera_System : public System {

	virtual void tick(f32 dt) override;
};

struct Sprite_System : public System {
	virtual void tick(f32 dt) override;
};

#define ALL_SYSTEMS(macro) \
macro(Camera_System) \
macro(Sprite_System)
