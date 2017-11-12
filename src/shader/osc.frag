R"(
#version 150
in vec4 color;
in vec4 t;

out vec4 f_color;

void main () {
	// calculate 2d gaussian function
	float alpha = exp(-(t.x*t.x*t.z + t.y*t.y*t.w));

	f_color = vec4(color.rgb, alpha);
}
)"
