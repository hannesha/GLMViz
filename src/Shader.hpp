#pragma once

#include <GL/glew.h>

class Shader {
	public:
		inline GLuint get_shader() const {return shader;};
		Shader(const char* , GLuint);
		~Shader();
	private:
		GLuint shader;
};
