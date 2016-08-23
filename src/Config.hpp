#pragma once

#include <stdlib.h>
#include <iostream>

#include <string>
#include <libconfig.h++>

class Config {
	public:
		Config();
		
		int w_aa = 4;
		int w_height = 1024;
		int w_width = 768;
		
		int duration = 100;
		int FS = 44100;
		int fps = 60;
		long long fft_size = 2<<13;
		long long output_size = 100;

		long long buf_size = FS * duration / 1000;		
		long long fft_output_size = fft_size/2+1;	
		float d_freq = (float) FS / (float) fft_output_size;
		float fft_scale = 1.0f/((float)(buf_size/2+1)*32768.0f);
		float slope = 0.5f;
		float offset = 1.0f;

		
		float top_color[4] = {211.0, 38.0, 46.0, 1.0};
		float bot_color[4] = {35.0, 36.0, 27.5, 1.0};
		float line_color[4] = {70.0, 72.0, 75.0, 1.0};
		float gradient = 1.0f;
		float gravity = 0.008f;		

		bool rainbow = false;	
		
		float bar_width = 0.75f;

	private:
		libconfig::Config cfg;
		void read_rgba(const std::string &path, float rgba[]);
};
