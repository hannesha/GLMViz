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
#include <vector>
#include <libconfig.h++>

#define MAX_OSCILLOSCOPES 4
#define MAX_SPECTRA 4

enum class Source {FIFO, PULSE};

class Config {
	public:
		Config(const std::string&);
		void reload();

		int w_aa = 4;
		int w_height = 1024;
		int w_width = 768;

		struct Input {
			Source source = Source::PULSE;
			std::string file = "/tmp/mpd.fifo";
			std::string device = "";
			bool stereo = false;
			long long f_sample = 44100;

			void parse(const std::string&, libconfig::Config&);
		};
		Input input;

		int duration = 50;
		int fps = 60;
		long long fft_size = 2<<12;
		long long buf_size = input.f_sample * duration / 1000;
		long long fft_output_size = fft_size/2+1;
		float d_freq = (float) input.f_sample / (float) fft_size;
		float fft_scale = 1.0f/((float)(buf_size/2+1)*32768.0f);

		struct Transformation {
			float Xmin = -1, Xmax = 1, Ymin = -1, Ymax = 1;

			void parse(const std::string&, libconfig::Config&);
		};

		struct Color {
			float rgba[4];

			void parse(const std::string&, libconfig::Config&);
			void normalize(const Color&);
			void normalize();
		};

		struct Oscilloscope {
			int channel = 0;
			float scale = 1;
			float width = 0.01;
			Color color = {211, 38, 46, 1};
			Transformation pos;

			void parse(const std::string&, libconfig::Config&);
		};

		struct Spectrum {
			int channel = 0;
			float min_db = -80, max_db = 0;
			float slope = 0.5f;
			float offset = 1.0f;
			int output_size = 100;

			Color top_color = {211, 38, 46, 1}; 		
			Color bot_color = {35, 36, 27, 1};
			Color line_color = {70, 72, 75, 1};
			Transformation pos;
			float gradient = 1.0f;
			float gravity = 8.0f;
			float bar_width = 0.75f;

			bool rainbow = false;
			Color freq_d = {1, 1, 1, 1};
			Color phase_d = {0, 0, 0, 1};
			bool dB_lines = true;

			void parse(const std::string&, libconfig::Config&);
			void clamp_output_size(const size_t fft_output_size){output_size = (size_t)output_size < fft_output_size ? output_size : fft_output_size; };
			void parse_rainbow(const std::string&, libconfig::Config&);
		};

		Oscilloscope osc_default;
		Spectrum spec_default;

		std::vector<Oscilloscope> oscilloscopes;
		std::vector<Spectrum> spectra;

	private:
		libconfig::Config cfg;
		std::string file;
};
