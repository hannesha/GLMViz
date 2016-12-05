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

	// fix feedback buffer resize hang
	float y_old = clamp(gravity_old, -0.5, 0.7);
	float yg = y_old - gravity * time_old * time_old;

	// fix shader gravity during track skip
	y = clamp(y, -0.5, 0.7);

	// elimitate branching using mix
	float gt_y = gt(y, yg);
	v_gravity = mix(y_old, y, gt_y);
	v_time = mix(time_old + 1.0, 0.0, gt_y);
	v_y = max(yg, y);
}
)"
