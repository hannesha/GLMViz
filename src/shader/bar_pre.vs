R"(
#version 150
// Complex freq amplitude
in vec2 a;
// old values from TF
in float gravity_old;
in float time_old;

// new TF values + current y value
out float v_gravity;
out float v_time;
out float v_y;

uniform float fft_scale;
uniform float slope;
uniform float offset;
uniform float gravity;

const float lg = 1 / log(10);

// greater than comparison function
float gt(float x, float y){
	return max(sign(x - y), 0.0);
}

void main(){
	// normalize fft output
	float a_norm = length(a) * fft_scale;
	// convert fft output into dB and calculate gravity
	float y = slope * log(a_norm) * lg + offset;
	float yg = gravity_old - gravity * time_old * time_old;

	// fix shader gravity during track skip
	y = clamp(y, -0.5, 0.5);

	// elimitate branching using mix
	v_gravity = mix(gravity_old, y, gt(y, yg));
	v_time = mix(time_old + 1.0, 0.0, gt(y, yg));
	v_y = mix(yg, y, gt(y, yg));
}
)"
