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

namespace GL {
	class Program {
		public:
			Program();
			~Program(){ glDeleteProgram(id); };
			Program(const Program&) = delete;
			Program(Program&& p): id(p.id){ p.id = 0; };
			Program& operator=(Program&&) = default;

			template<typename ... T> void link(T&... shs){
				attach(shs...);
				glLinkProgram(id);
				detach(shs...);
				check_link_status();
			}

			template<typename ... T> void link_TF(const size_t n, const char** varyings, T&... shs){
				glTransformFeedbackVaryings(id, n, varyings, GL_INTERLEAVED_ATTRIBS);
				link(shs...);
			}
			void check_link_status();

			inline void use(){ glUseProgram(id); };
			inline GLuint get_id() const { return id; };
			inline void operator()(){ use(); };
			inline GLint get_uniform(const char* name) const { return glGetUniformLocation(id, name); };
			inline GLint get_attrib(const char* name) const { return glGetAttribLocation(id, name); };
		private:
			GLuint id;

			template<typename ... T> inline void attach(Shader& sh, T& ... shs){ attach(sh); attach(shs...); };
			inline void attach(Shader& sh){ glAttachShader(id, sh.id); };
			template<typename ... T> inline void detach(Shader& sh, T& ... shs){ detach(sh); detach(shs...); };
			inline void detach(Shader& sh){ glDetachShader(id, sh.id); };
	};
}
