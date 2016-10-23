R"(
#version 150

// sinebow fragment shader
in vec4 color;

out vec4 f_color;

const float f2pi = radians(360.0);
const vec3 pi2 = vec3(f2pi, f2pi, f2pi);

// phase shifts for the cos function
const vec3 phase = vec3(radians(0.0), radians(120.0), radians(240.0));
const vec3 scale = vec3(0.5, 0.5, 0.5);
const vec3 offset = vec3(0.5, 0.5, 0.5);

// gamma correction
const vec3 gamma = vec3(2.2);
void main () {
	vec3 ccos = cos(pi2 * color.rgb + phase);
	vec3 rb_color = scale * ccos + offset;

	// apply gamma correction
	f_color.rgb = pow(rb_color, gamma);
	f_color.a = color.a;
}
)"
