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

void main(){
	float a_norm = length(a) * fft_scale;
	float y = slope * log(a_norm) * lg + offset;
	float yg = gravity_old - gravity * time_old * time_old;
	if(y > yg){
		v_gravity = y;
		v_time = 0.0;
	}else{
		v_gravity = gravity_old;
		v_time = time_old + 1.0;
		y = yg;
	}
	v_y = y;
}
)"
