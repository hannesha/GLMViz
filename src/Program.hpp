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

#include <GL/glew.h>
#include <vector>
#include "Shader.hpp"
#include "Config.hpp"

class Program {
	public:
		Program();
		~Program();
		void attach_shader(const Shader& s);
		void link();
		void link_TF(const size_t, const char**);
		inline void use(){glUseProgram(program_id);};
		GLuint get_id() const;
		GLint get_uniform(const char*) const;
		GLint get_attrib(const char*) const;
	private:
		GLuint program_id;
		std::vector<GLuint> shaders;
};

void init_bar_shader(Program&, Config&);
void init_line_shader(Program&);
void init_bar_gravity_shader(Program&);
