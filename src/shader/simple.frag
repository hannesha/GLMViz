R"(
#version 150

in vec4 color;

out vec4 f_color;

const vec3 gamma = vec3(2.2);
void main () {
	f_color.xyz = pow(color.xyz, gamma);
	f_color.w = color.w;
}
)"
