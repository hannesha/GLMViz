#include "GL_utils.hpp"
#include <system_error>
#include <string>
#include <vector>
#include <iostream>

using namespace GL;

std::error_code init_error = std::make_error_code(std::errc::protocol_error);

Program::Program() {
	id = glCreateProgram();

	if(id == 0) {
		throw std::system_error(init_error, "Failed to create shader program!");
	}
}

void Program::check_link_status() {
	// check link status and throw if the shaders can't be linked
	GLint link_status;
	glGetProgramiv(id, GL_LINK_STATUS, &link_status);

	if(link_status == GL_FALSE) {
		GLint length;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> log(length);
		glGetProgramInfoLog(id, length, &length, log.data());

		std::string err(log.begin(), log.end());
		throw std::invalid_argument(err);
	}
}

Shader::Shader(const char* code, GLuint type) {
	id = glCreateShader(type);

	if(id == 0){
		throw std::system_error(init_error, "Failed to create shader!");
	}

	glShaderSource(id, 1, &code, nullptr);
	glCompileShader(id);

	GLint status;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);

	if(status == GL_FALSE) {
		GLint length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> log(length);
		glGetShaderInfoLog(id, length, &length, log.data());

		std::string err(log.begin(), log.end());
		//std::cout << err << std::endl;
		throw std::invalid_argument(err);
	}
}

void GL::get_error(const char* str, const char* function) {
	GLenum err = glGetError();
	std::string err_str;

	switch(err) {
	case GL_INVALID_OPERATION:
		err_str = "INVALID_OPERATION";
		break;

	case GL_INVALID_ENUM:
		err_str = "INVALID_ENUM";
		break;

	case GL_INVALID_VALUE:
		err_str = "INVALID_VALUE";
		break;

	case GL_OUT_OF_MEMORY:
		err_str = "OUT_OF_MEMORY";
		break;

	case GL_INVALID_FRAMEBUFFER_OPERATION:
		err_str = "INVALID_FRAMEBUFFER_OPERATION";
		break;

	default:
		return;
	}

	if(!function){
		std::cout << str << " :" << err_str << std::endl;
	}else{
		std::cout << str << " in: " << function << " :" << err_str << std::endl;
	}

}

void GL::init() {
#ifdef USE_GLEW
	glewExperimental = GL_TRUE;
	GLenum status = glewInit();

	if(status != GLEW_OK){
		throw std::system_error(init_error, "GLEW init failed");
	}
#endif
}
