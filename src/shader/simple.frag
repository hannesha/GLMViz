R"(
#version 150
/* simple frag shader with gamma correction */
in vec4 color;

out vec4 f_color;

const vec3 gamma = vec3(2.2);
void main () {
	f_color.rgb = pow(color.rgb, gamma);
	f_color.a = color.a;
}
)"
