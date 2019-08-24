#pragma once

#include <ch_stl/types.h>

struct System {
	bool is_enabled = true;

	virtual void tick(f32 dt) {}
};

struct Sprite_System : public System {
	virtual void tick(f32 dt) override;
};

#define ALL_SYSTEMS(macro) \
macro(Sprite_System)
