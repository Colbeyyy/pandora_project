#pragma once

#include <ch_stl/math.h>

struct Component {
	virtual void on_created() {}
	virtual ~Component() {}
};

struct Transform : public Component {
	ch::Vector2 position;
};

struct Motion : public Component {
	ch::Vector2 velocity;
	ch::Vector2 acceleration;
};

