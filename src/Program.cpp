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

#include "Program.hpp"

namespace GL {

	Program::Program(){
		id = glCreateProgram();
	}

	Program::~Program(){
		glDeleteProgram(id);
	}

	void Program::link(std::initializer_list<const std::reference_wrapper<GL::Shader>> shaders){
		// attach compiled shaders
		for (GL::Shader& sh : shaders){
			glAttachShader(id, sh.id);
		}

		glLinkProgram(id);

		// detach shaders for later cleanup
		for (GL::Shader& sh : shaders){
			glDetachShader(id, sh.id);
		}
	}

	void Program::link_TF(const size_t n, const char** fb_varyings, std::initializer_list<const std::reference_wrapper<GL::Shader>> shaders){
		// link with transform feedback varyings
		// set TF varyings
		glTransformFeedbackVaryings(id, n, fb_varyings, GL_INTERLEAVED_ATTRIBS);
		//std::cout << glGetError() << std::endl;

		link(shaders);
	}
}
