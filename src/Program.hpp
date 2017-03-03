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

#include "GL_utils.hpp"
#include <initializer_list>
#include <functional>

namespace GL {
	class Program {
		public:
			Program();
			~Program();

			void link(std::initializer_list<const std::reference_wrapper<GL::Shader>>);
			void link_TF(const size_t, const char**, std::initializer_list<const std::reference_wrapper<GL::Shader>>);

			inline void use(){ glUseProgram(id); };
			inline void operator()(){ use(); };
			inline GLint get_uniform(const char* name) const { return glGetUniformLocation(id, name); };
			inline GLint get_attrib(const char* name) const { return glGetAttribLocation(id, name); };
		private:
			GLuint id;
	};
}
