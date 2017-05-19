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

#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>

namespace GL {
	// VBO RAII wrapper
	class Buffer{
		public:
			inline Buffer() { glGenBuffers(1, &id); };
			inline ~Buffer() { glDeleteBuffers(1, &id); };
			// disable copying
			Buffer(const Buffer&) = delete;
			Buffer(Buffer&& b):id(b.id){ b.id = 0; };
			Buffer& operator=(Buffer&&) = default;

			inline void bind() { glBindBuffer(GL_ARRAY_BUFFER, id); };
			static inline void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); };
			inline void operator()(){ bind(); };
			inline void tfbind() { glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, id); };
			GLuint id;
	};

	// VAO RAII wrapper
	class VAO{
		public:
			inline VAO() { glGenVertexArrays(1, &id); };
			inline ~VAO() { glDeleteVertexArrays(1, &id); };
			// disable copying
			VAO(const VAO&) = delete;
			VAO(VAO&& v):id(v.id){ v.id = 0; };
			VAO& operator=(VAO&&) = default;

			inline void bind() { glBindVertexArray(id); };
			static inline void unbind() { glBindVertexArray(0); };
			inline void operator()(){ bind(); };
			GLuint id;
	};

	// Shader RAII wrapper
	class Shader {
		public:
			inline Shader(const char* code, GLuint type){
				id = glCreateShader(type);
				glShaderSource(id, 1, &code, nullptr);
				glCompileShader(id);
			};
			inline ~Shader(){ glDeleteShader(id); };
			// disable copying
			Shader(const Shader&) = delete;
			Shader(Shader&& s):id(s.id){ s.id = 0; };
			Shader& operator=(Shader&&) = default;

			GLuint id;
	};
}
