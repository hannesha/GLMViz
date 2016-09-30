R"(
#version 150

in vec2 pos;

out vec4 color;

uniform float slope;
uniform float offset;
uniform vec4 line_color;
uniform mat4 trans;

//const float div255 = 1.0/255.0;
//const vec4 n_color = vec4(div255, div255, div255, 1.0);
void main () {
	color = line_color;
	// limit line range from 1 to -1
	gl_Position = trans * vec4 (pos.x, clamp(slope * pos.y + offset, -1.0, 1.0) , 0.0, 1.0);
}
)"
