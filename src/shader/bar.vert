R"(
#version 330

//in float x;
layout(location = 0) in float y;

uniform vec4 bot_color;
uniform vec4 top_color;

// switch gradient, 0:full range per bar, 1:0dB has top_color
uniform float gradient;
uniform float length_1;

out vec4 v_bot_color;
out vec4 v_top_color;

void main () {
	float y_clamp = clamp(y * 2.0 , -1.0, 1.0);
	// calculate x coordinates
	float x = mix(-1., 1., (float(gl_VertexID) + 0.5) * length_1);
	
	gl_Position = vec4(x, y_clamp, 0.0, 1.0);
	v_bot_color = bot_color;

	// calculate normalized top color	
	y_clamp = mix(1.0, y_clamp, gradient);
	v_top_color = mix(bot_color, top_color, y_clamp * 0.5 + 0.5);
}
)"
