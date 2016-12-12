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

#include "FFT.hpp"
#include "Config.hpp"
#include "Program.hpp"

class Spectrum {
	public:
		Spectrum(Config&);
		~Spectrum(){};

		void draw();
		void update_fft(FFT&);
		void configure(Config&);
		void resize(const size_t);
		void set_transformation(const double, const double);
	private:
		Program sh_bars, sh_bars_pre, sh_lines;
		GL::VAO v_bars[2], v_bars_pre[2], v_lines;
		GL::Buffer b_x, b_fft, b_fb[2], b_lines;
		unsigned tf_index = 0;
		size_t output_size;
		bool draw_lines;

		const float dB_lines[36] = {
			-1.0,  0.0, 1.0,  0.0, //   0dB
			-1.0, -0.5, 1.0, -0.5, // -10dB
			-1.0, -1.0, 1.0, -1.0, // -20dB
			-1.0, -1.5, 1.0, -1.5, // -30dB
			-1.0, -2.0, 1.0, -2.0, // -40dB
			-1.0, -2.5, 1.0, -2.5, // -50dB
			-1.0, -3.0, 1.0, -3.0, // -60dB
			-1.0, -3.5, 1.0, -3.5, // -70dB
			-1.0, -4.0, 1.0, -4.0  // -80dB
		};

		void init_bars();
		void init_bars_pre();
		void init_lines();
		void fill_tf_buffers(const size_t);
		void update_x_buffer(const size_t);
		void resize_fft(const size_t);
};
