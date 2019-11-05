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

uniform float time;
uniform vec2 screen_size;

float distance_from_sphere(in vec3 point, in vec3 center, float radius) {
	return length(point - center) - radius;
}

#define NUM_STEPS 100
#define MIN_HIT_DISTANCE = 0.001
#define MAX_TRACE_DISTANCE = 1000.0
#define PI 3.1415926535
#define TAU (PI * 2.0)

vec3 ray_march(in vec3 orig, in vec3 dir) {
	float distance = 0.0;


	return vec3(0.0);
}


void main() {

	vec2 uv = out_uv.xy * 2.0 - 1.0;

	frag_color = vec4(out_uv, (sin(time * PI) + 1.0) / 2.0, 1);
}
#endif