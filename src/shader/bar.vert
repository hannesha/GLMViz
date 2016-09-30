R"(
#version 150

in float x;
in float y;

uniform vec4 bot_color;
uniform vec4 top_color;

// switch gradient, 0:full range per bar, 1:0dB has top_color
uniform float gradient;

out vec4 v_bot_color;
out vec4 v_top_color;

//const float div255 = 1.0/255.0;
//const vec4 n_color = vec4(div255, div255, div255, 1.0);
void main () {
	float y_clamp = clamp(y * 2.0 , -1.0, 1.0);
	//vec4 n_b_color = bot_color * n_color;
	//vec4 n_t_color = top_color * n_color;
	
	gl_Position = vec4(x, y_clamp, 0.0, 1.0);
	v_bot_color = bot_color;

	// calculate normalized top color	
	y_clamp = mix(1.0, y_clamp, gradient);
	v_top_color = mix(bot_color, top_color, y_clamp * 0.5 + 0.5);
}
)"
