#include "sprite_renderer.h"
#include "draw.h"

Sprite_Renderer sprite_renderer;

usize Sprite_Renderer::push(ch::Vector2 position, Sprite sprite) {
	Render_Command rc(position, sprite);
	return commands.push(rc);
}

void Sprite_Renderer::flush() {
	if (!commands.count) return;

	Shader* s = find_shader(CH_TEXT("image"));
	s->bind();
	Texture* t = commands[0].sprite.atlas;
	t->set_active();
	refresh_transform();

	imm_begin();
	for (Render_Command& it : commands) {
		if (it.sprite.atlas != t) {
			imm_flush();
			t = it.sprite.atlas;
			t->bind();
			imm_begin();
		}

		const f32 x0 = it.position.x - (f32)(it.sprite.width / 2);
		const f32 y0 = it.position.y - (f32)(it.sprite.height / 2);
		const f32 x1 = x0 + (f32)it.sprite.width;
		const f32 y1 = y0 + (f32)it.sprite.height;
		imm_sprite(x0, y0, x1, y1, ch::white, it.sprite);
	}
	imm_flush();

	commands.count = 0;
}
