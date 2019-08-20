#pragma once

#include <ch_stl/math.h>

enum Component_Type {
	CT_None,
};

struct Component {
	
};

struct Transform : Component {
	ch::Vector2 position;
	f32 rotation;
};

struct Camera : public Component {
	f32 ortho_size;
};