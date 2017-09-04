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

#ifndef NDEBUG
#include <vector>
#include <string>
#include <iostream>
#endif
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
			inline void bind(GLenum target) { glBindBuffer(target, id); };
			inline void operator()(){ bind(); };
			inline void operator()(GLenum target){ bind(target); };
			inline void tfbind() { glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, id); };

			static inline void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); };
			static inline void unbind(GLenum target) { glBindBuffer(target, 0); };
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

#ifndef NDEBUG
				GLint status;
				glGetShaderiv(id, GL_COMPILE_STATUS, &status);
				if(status == GL_FALSE){
					GLint length;
					glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

					std::vector<GLchar> log(length);
					glGetShaderInfoLog(id, length, &length, log.data());

					std::string err(log.begin(), log.end());
					std::cout << err << std::endl;
				}
#endif
			};
			inline ~Shader(){ glDeleteShader(id); };
			// disable copying
			Shader(const Shader&) = delete;
			Shader(Shader&& s):id(s.id){ s.id = 0; };
			Shader& operator=(Shader&&) = default;

			GLuint id;
	};

	// Texture RAII wrapper
	class Texture{
		public:
			inline Texture() { glGenTextures(1, &id); };
			inline ~Texture() { glDeleteTextures(1, &id); };
			// disable copying
			Texture(const Texture&) = delete;
			// move constructor
			Texture(Texture&& t):id(t.id){ t.id = 0; };
			Texture& operator=(Texture&&) = default;

			inline void bind(GLenum target) { glBindTexture(target, id); };
			static inline void unbind(GLenum target) { glBindTexture(target, 0); };
			inline void operator()(GLenum target){ bind(target); };
			GLuint id;
	};

	// Framebuffer RAII wrapper
	class FBO{
		public:
			inline FBO() { glGenFramebuffers(1, &id); };
			inline ~FBO() { glDeleteFramebuffers(1, &id); };
			// disable copying
			FBO(const FBO&) = delete;
			// move constructor
			FBO(FBO&& t):id(t.id){ t.id = 0; };
			FBO& operator=(FBO&&) = default;

			inline void bind(GLenum target) { glBindFramebuffer(target, id); };
			inline void bind() { bind(GL_FRAMEBUFFER); };
			static inline void unbind(GLenum target) { glBindFramebuffer(target, 0); };
			static inline void unbind() { unbind(GL_FRAMEBUFFER); };
			inline void operator()(GLenum target){ bind(target); };
			inline void operator()(){ bind(); };
			GLuint id;
	};
}
