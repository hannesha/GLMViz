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

#include "Buffer.hpp"
#include "Config.hpp"
#include "Program.hpp"

class Oscilloscope {
	public:
		Oscilloscope(Config&);
		~Oscilloscope(){};

		void draw(const size_t);
		void update_x_buffer(const size_t);
		void update_buffer(Buffer<int16_t>&);
		void set_uniforms(Config&);
	private:
		Program sh_crt;
		GL::VAO v_crt;
		GL::Buffer b_crt_x, b_crt_y;

		void init_crt();
};
