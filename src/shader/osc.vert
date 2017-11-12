R"(
#version 150
in float y;

uniform float length_1; // 1/length
uniform float scale;

void main(){
	// calculate x coordinate
	float x = mix(-1., 1., (float(gl_VertexID) + 0.5) * length_1);
	gl_Position = vec4(x, clamp(y * scale, -1.0, 1.0), 0.0, 1.0);
}
)"
