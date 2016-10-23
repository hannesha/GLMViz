/*
 *	GLMViz is a OpenGL based Visualizer for mpd.
 *	Copyright (C) 2016  Hannes Haberl
 *	
 *	This file is part of GLMViz.
 *
 *	GLMViz is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	GLMViz is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with GLMViz.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include <libconfig.h++>

enum class Source {FIFO, PULSE};

class Config {
	public:
		Config();

		int w_aa = 4;
		int w_height = 1024;
		int w_width = 768;

		Source source = Source::PULSE;
		std::string fifo_file = "/tmp/mpd.fifo";
		int duration = 50;
		long long FS = 44100;
		int fps = 60;
		long long fft_size = 2<<12;
		long long output_size = 100;

		long long buf_size = FS * duration / 1000;
		long long fft_output_size = fft_size/2+1;
		float d_freq = (float) FS / (float) fft_size;
		float fft_scale = 1.0f/((float)(buf_size/2+1)*32768.0f);
		float slope = 0.5f;
		float offset = 1.0f;
		
		float top_color[4] = {211.0, 38.0, 46.0, 1.0};
		float bot_color[4] = {35.0, 36.0, 27.5, 1.0};
		float line_color[4] = {70.0, 72.0, 75.0, 1.0};
		float gradient = 1.0f;
		float gravity = 0.1f;

		bool rainbow = false;
		
		float bar_width = 0.75f;
		bool draw_dB_lines = true;
	private:
		libconfig::Config cfg;
		void read_rgba(const std::string& path, float rgba[]);
		void read_rainbow(const std::string& path);
		void to_srgb(float rgba[]);

		float max_db = 0.0f;
		float min_db = -80.0f;

		float rb_phase[3] = {0.0, 0.0, 0.0};
		float rb_freq[3]  = {1.0, 1.0, 1.0};
};
