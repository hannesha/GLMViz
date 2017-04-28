/*
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
#include <memory>
#include <array>

class Spectrum {
	public:
		Spectrum(const Config::Spectrum&, const unsigned);
		// disable copy construction
		Spectrum(const Spectrum&) = delete;
		~Spectrum(){};

		void draw();
		void update_fft(FFT&);
		void update_fft(std::vector<std::shared_ptr<FFT>>&);
		void configure(const Config::Spectrum&);

	private:
		GL::Program sh_bars_pre, sh_lines;
		std::array<GL::Program, 2> sh_bars;

		GL::VAO v_lines;
		std::array<GL::VAO, 2> v_bars, v_bars_pre;

		GL::Buffer b_x, b_fft, b_lines;
		std::array<GL::Buffer, 2> b_fb;
		unsigned tf_index = 0;
		size_t output_size, offset;
		bool draw_lines;
		unsigned id, bar_shader_id, channel;

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

		void init_bar_shader();
		void init_bars();
		void init_bar_pre_shader();
		void init_bars_pre();
		void init_line_shader();
		void init_lines();

		void resize_tf_buffers(const size_t);
		void resize_x_buffer(const size_t);
		void resize_fft_buffer(const size_t);
		void resize(const size_t);
		void set_transformation(const Config::Transformation&);
};
