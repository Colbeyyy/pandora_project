#include "console.h"
#include "draw.h"
#include "input.h"
#include "game.h"

#include <ch_stl/math.h>

enum Console_State {
	CS_Closed,
	CS_Small,
	CS_Full,
	CS_MAX
};

static u32 console_state;
static f32 current_height;
static f32 target_height;

static Shader* console_shader;

static const f32 open_speed = 5.f;

void init_console() {
	console_shader = find_shader(CH_TEXT("solid_shape"));
}

void tick_console(f32 dt) {

	if (was_key_pressed(CH_KEY_TILDE)) {
		console_state += 1;
		if (console_state == Console_State::CS_MAX) console_state = 0;

		switch(console_state) {
		case Console_State::CS_Closed:
			target_height = 0.f;
			break;
		case Console_State::CS_Small:
			target_height = 30.f;
			break;
		case Console_State::CS_Full:
			target_height = 400.f;
			break;
		case Console_State::CS_MAX:
			invalid_code_path;
			break;
		}
	}

	current_height = ch::interp_to(current_height, target_height, dt, open_speed);
}

void draw_console() {
	assert(console_shader);
	console_shader->bind();
	const ch::Vector2 viewport_size = the_window.get_viewport_size();
	const f32 width = (f32)viewport_size.ux;

	render_right_handed();

	imm_begin();
	const f32 x0 = 0.f;
	const f32 y0 = 0.f;
	const f32 x1 = x0 + width;
	const f32 y1 = y0 - current_height;
	imm_quad(x0, y0, x1, y1, ch::white);
	imm_flush();
}