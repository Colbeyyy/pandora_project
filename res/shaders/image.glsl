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

	out_position = position;
	out_color = color;
	out_uv = uv;
	out_normal = normal;
}
#endif

#ifdef FRAGMENT
const float ambient_strength = 0.1;
const vec3 light_color = vec3(1);
const vec2 light_pos = vec2(0, 0);

out vec4 frag_color;
in vec2 out_position;
in vec4 out_color;
in vec2 out_uv;
in vec2 out_normal;

uniform sampler2D ftex;

void main() {
	vec4 tex_color = texture(ftex, out_uv);

	float constant = 1.0;
	float linear = 0.009;
	float quadratic = 0.00032;

	float distance = length(light_pos - out_position);
	float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

	vec3 diffuse = light_color * attenuation;
	vec3 ambient = light_color * ambient_strength;

	vec3 result = (ambient + diffuse) * out_color.rgb;

	frag_color = vec4(result, 1) * tex_color;
}
#endif