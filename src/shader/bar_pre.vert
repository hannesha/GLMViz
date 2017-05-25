R"(
#version 150
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

// fft texture buffer
uniform samplerBuffer tbo_fft;
uniform vec3 log_params; //{ a, b, log_switch}

const float lg = 1 / log(10);

// greater than comparison function
float gt(float x, float y){
	return max(sign(x - y), 0.0);
}

void main(){
	// fetch fft output
	float a;
	if(log_params.z > 0.0){
		// log fetch
		float i = log_params.x * exp(log_params.y * gl_VertexID);
		int i_f = int(floor(i));
		// fetch both indices
		float a_1 = length(texelFetch(tbo_fft, i_f).xy) * fft_scale;
		float a_2 = length(texelFetch(tbo_fft, i_f + 1).xy) * fft_scale;
		// interpolate fft values
		a = mix(a_1, a_2, i - float(i_f));
	}else{
		// linear fetch
		a = length(texelFetch(tbo_fft, gl_VertexID).xy) * fft_scale;
	}

	// normalize fft output
	//float a_norm = a * fft_scale;

	// convert fft output into dB
	float y = slope * log(a) * lg + offset;

	// clamp values
	float y_old = clamp(gravity_old, -0.5, 0.7);
	y = clamp(y, -0.5, 0.7);

	// calculate gravity
	float yg = y_old - gravity * time_old * time_old;

	// set output variables
	float gt_y = gt(y, yg);
	v_gravity = mix(y_old, y, gt_y);
	v_time = mix(time_old + 1.0, 0.0, gt_y);
	v_y = max(yg, y);
}
)"
