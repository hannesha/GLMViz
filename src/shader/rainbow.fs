R"(
#version 150

// sinebow fragment shader
in vec4 color;

out vec4 f_color;

const float f2pi = radians(360.0);
const vec4 pi2 = vec4(f2pi, f2pi, f2pi, 0.0);

// phase shifts for the cos function
const vec4 phase = vec4(radians(0.0), radians(120.0), radians(240.0), 0.0);
const vec4 scale = vec4(0.5, 0.5, 0.5, 0.0);
const vec4 offset = vec4(0.5, 0.5, 0.5, 1.0);

void main () {
	vec4 ccos = cos(pi2 * color + phase);
	f_color = scale * ccos + offset;
}
)"
