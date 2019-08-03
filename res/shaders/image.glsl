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
out vec2 out_lp;
void main() {
    gl_Position =  projection * view * vec4(position, -z_index, 1.0);

	out_lp = (projection * view * vec4(0, 100, 0, 0)).xy;

	out_position = position;
	out_color = color;
	out_uv = uv;
}
#endif

#ifdef FRAGMENT
const float ambient_strength = 0.3;
const vec3 light_color = vec3(1);

out vec4 frag_color;
in vec2 out_position;
in vec4 out_color;
in vec2 out_uv;
in vec2 out_normal;
in vec2 out_lp;
uniform sampler2D ftex;
void main() {
	vec4 tex_color = texture(ftex, out_uv);

	vec2 norm = normalize(out_normal);
	vec2 distance_from_light = out_lp - out_position;
	vec2 light_dir = normalize(distance_from_light);

	float d = max(length(distance_from_light / 1000), 0);

	float diff = max(dot(norm, light_dir), d);

	vec3 diffuse = light_color * diff;
	vec3 ambient = light_color * ambient_strength;

	vec3 result = (ambient + diffuse) * out_color.rgb;

	frag_color = vec4(result, 1) * tex_color;
}
#endif