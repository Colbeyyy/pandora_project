#ifdef VERTEX
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec2 normal;
layout(location = 4) in float z_index;
uniform mat4 projection;
uniform mat4 view;
out vec2 out_position;
out vec4 out_color;
out vec2 out_uv;
out vec2 out_normal;
void main() {
    gl_Position =  projection * view * vec4(position, -z_index, 1.0);

	out_color = color;
	out_uv = uv;
	out_normal = normal;
}
#endif

#ifdef FRAGMENT

out vec4 frag_color;
in vec2 out_position;
in vec4 out_color;
in vec2 out_uv;
in vec2 out_normal;

uniform sampler2D ftex;

void main() {
	vec4 tex_color = texture(ftex, out_uv);

	frag_color = tex_color * out_color;
}
#endif