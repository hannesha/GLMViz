R"(
#version 150
// old values from TF
in float time_old;
in float y_old;

// new TF values + current y value
out float v_time;
out float v_y;

uniform float fft_scale;
uniform float slope;
uniform float offset;
uniform float gravity;

// fft texture buffer
uniform samplerBuffer tbo_fft;
uniform vec3 log_params; //{ a, b, log_switch}
uniform float dt;

const float lg = 1. / log(10.);

// greater than comparison function
float gt(float x, float y){
	return max(sign(x - y), 0.0);
}

float acc(float t){
	return -gravity * t;
}

void main(){
	// fetch fft output
	float a;
	if(log_params.z > 0.0){
		// log fetch
		float i = log_params.x * exp(log_params.y * gl_VertexID);
		int i_f = int(floor(i));
		// fetch both indices and tangents
		float a_0 = length(texelFetch(tbo_fft, i_f - 1).xy) * fft_scale;
		float a_1 = length(texelFetch(tbo_fft, i_f).xy) * fft_scale;
		float a_2 = length(texelFetch(tbo_fft, i_f + 1).xy) * fft_scale;
		float a_3 = length(texelFetch(tbo_fft, i_f + 2).xy) * fft_scale;
		float t0 = a_1 - a_0;
		float t1 = a_3 - a_2;
		// interpolate fft values
		float x = fract(i);
		float x2 = x * x;
		float x3 = x2 * x;
		// cubic interpolation
		a = (2. * x3 -3. * x2 +1.) * a_1 + (x3 -2. * x2 +x) * t0 +
			(-2. * x3 +3. * x2) * a_2 + (x3 - x2) * t1;
	}else{
		// linear fetch
		a = length(texelFetch(tbo_fft, gl_VertexID).xy) * fft_scale;
	}

	// convert fft output into dB
	float y = slope * log(a) * lg + offset;

	// clamp values
	float y_o = clamp(y_old, -0.5, 0.7);
	y = clamp(y, -0.5, 0.7);
	float time = max(time_old, 0.0);

	// RK4 integration
	float k1 = acc(time);
	float k2 = acc(time + dt * 0.5);
	float k4 = acc(time + dt);
	float dydt = 1./6. * (k1 + 4. * k2 + k4);

	// calculate gravity
	float yg = y_o + dydt * dt;
	// update time
	time = time + dt;

	// set output variables
	float gt_y = gt(y, yg);
	v_time = mix(time, 0., gt_y);
	v_y = max(yg, y);
}
)"
