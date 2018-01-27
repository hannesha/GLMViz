/*
 *	Copyright (C) 2016,2017  Hannes Haberl
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
#include <tuple>

namespace Module_Config {
	enum class Source {FIFO, PULSE};

	struct Input {
		Source source = Source::PULSE;
		std::string file = "/tmp/mpd.fifo";
		std::string device = "";
		bool stereo = false;
		long long f_sample = 44100;
		long long latency = 1100; // f_sample * s_latency(0.025 s)

		inline bool operator==(const Input& rhs) const{
			return std::tie(source, stereo, f_sample)
				== std::tie(rhs.source, rhs.stereo, rhs.f_sample);
		}
	};

	struct FFT {
		long long size = 1<<12;
		size_t output_size = size/2+1;
		float scale = 2.76678e-08;
		float d_freq = 44100./(float) size;
	};

	struct Transformation {
		float Xmin = -1, Xmax = 1, Ymin = -1, Ymax = 1;
	};

	struct Color {
		float rgba[4];

		inline void normalize(const Color& c){
			std::copy(c.rgba, c.rgba + 4, rgba);
			normalize();
		}
		inline void normalize(){
			// convert screen color(CRT gamma) to sRGB
			for(int i = 0; i < 3; i ++){
				rgba[i] = rgba[i] / 255;
			}
		}
	};

	struct Oscilloscope {
		int channel = 0;
		float scale = 1;
		float width = 0.01;
		float sigma = 4;
		float sigma_coeff = 2;
		Color color = {1, 1, 1, 1};
		Transformation pos;
	};

	struct Spectrum {
		int channel = 0;
		float min_db = -60, max_db = -5;
		float scale = 2.76678e-08;
		float slope = 0.5;
		float offset = 1.0;
		int output_size = 100;
		int data_offset = 0;
		float log_start = 5;
		float log_enabled = 0;

		Color top_color = {1, 1, 1, 1};
		Color bot_color = {1, 1, 1, 1};
		Color line_color = {0.57, 0.57, 0.57, 1};
		Transformation pos;
		float gradient = 1.0;
		float gravity = 8.0;
		float bar_width = 0.5;

		bool rainbow = false;
		Color freq_d = {1, 1, 1, 1};
		Color phase_d = {0, 0, 0, 1};
		bool dB_lines = false;
	};
}
