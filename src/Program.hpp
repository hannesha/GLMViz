#pragma once

#include <GL/glew.h>
#include <vector>
#include "Shader.hpp"

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

void init_bar_shader(Program&);
void init_line_shader(Program&);
void init_bar_gravity_shader(Program&);
