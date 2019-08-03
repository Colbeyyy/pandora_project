#ifdef VERTEX
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;
layout(location = 3) in float z_index;
uniform mat4 projection;
uniform mat4 view;
out vec4 out_color;
out vec2 out_uv;
void main() {
    gl_Position =  projection * view * vec4(position, -z_index, 1.0);
	out_color = color;
	out_uv = uv;
}
#endif

#ifdef FRAGMENT
out vec4 frag_color;
in vec4 out_color;
in vec2 out_uv;
uniform sampler2D ftex;
void main() {
	vec4 result = texture(ftex, out_uv);
	frag_color = result * out_color;
}
#endif