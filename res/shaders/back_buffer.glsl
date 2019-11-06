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

float distance_from_sphere(vec3 point, vec3 center, float radius) {
	return length(point - center) - radius;
}

float distance_from_box(vec3 point, vec3 bounds) {
	vec3 q = abs(point) - bounds;
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float smin(float a, float b, float k) {
	float h = max(k - abs(a - b), 0.0) / k;
	return min(a, b) - h * h * k * (1.0 / 4.0);
}

float test_the_world(in vec3 pos) {	
	float scale = 5.0;
	
	vec3 raw = pos;
	raw.x += time;
	raw.y += time;
	raw.y += time;

	vec3 up = vec3(1.0, 0.0, 0.0);
	

	float strength = 0.2;
	float displacement = (sin(scale * raw.x) * sin(scale * raw.y) * sin(scale * raw.z)) * strength;

	float time_speed = 2.0;

	float sphere1 = distance_from_sphere(pos, vec3(sin(time * time_speed) * 2, cos(time * time_speed) * 3, 0.0), 1.0);

	float sphere2 = distance_from_sphere(pos, vec3(sin(time * time_speed) * 1.5, cos(time * time_speed) * 1.5, 0.0), 1.0) + displacement;
	float box = distance_from_box(pos, vec3(1.0));

	// return box + displacement;

	return smin(sphere1, sphere2, 1);
}

vec3 calculate_normal(in vec3 pos) {
	const vec3 small_step = vec3(0.01, 0.0, 0.0);

    float gradient_x = test_the_world(pos + small_step.xyy) - test_the_world(pos - small_step.xyy);
    float gradient_y = test_the_world(pos + small_step.yxy) - test_the_world(pos - small_step.yxy);
    float gradient_z = test_the_world(pos + small_step.yyx) - test_the_world(pos - small_step.yyx);

    vec3 normal = vec3(gradient_x, gradient_y, gradient_z);

    return normalize(normal);
}

#define NUM_STEPS 1000
#define MIN_HIT_DISTANCE 0.01
#define MAX_TRACE_DISTANCE 1000.0
#define PI 3.1415926535
#define TAU (PI * 2.0)

vec4 ray_march(in vec3 orig, in vec3 dir) {
	float distance = 0.0;

	for (int i = 0; i < NUM_STEPS; i++) {
		vec3 pos = orig + distance * dir;

		float to_closest = test_the_world(pos);

		if (to_closest < MIN_HIT_DISTANCE) {
			vec3 normal = calculate_normal(pos);
			vec3 light_orig = vec3(2.0, -5.0, 3.0);
			vec3 light_dir = normalize(pos - light_orig);

			float diffuse_intensity = max(0.0, dot(normal, light_dir));
			vec3 ambient = vec3(0.3);

			float reflection_intensity = max(0.0, dot(normal, normalize(orig - pos)));
			vec3 color = mix(vec3(0.0, 0.7, 0.8), vec3(1.0, 0.3, 1.0), reflection_intensity);

			vec3 final = mix(ambient, color, diffuse_intensity);

			return vec4(final, 1.0);
		}

		if (distance > MAX_TRACE_DISTANCE) {
			break;
		}
		distance += to_closest;
	}

	return vec4(0.0);
}

void main() {
	vec2 uv = out_uv.xy * 2.0 - 1.0;

	vec3 cam_pos = vec3(0, 1, -5);
	vec3 ray_orig = cam_pos;
	vec3 ray_dir = vec3(uv, 1.0);

	frag_color = ray_march(ray_orig, ray_dir);
}
#endif