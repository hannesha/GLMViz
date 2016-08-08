#include "Shader.hpp"

Shader::Shader(const char* code, GLuint type){
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, nullptr);
	glCompileShader(shader);
}

Shader::~Shader(){
	glDeleteShader(shader);
}
