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

#include "Program.hpp"

Program::Program(){
	program_id = glCreateProgram();
}

Program::~Program(){
	glDeleteProgram(program_id);
}

void Program::link(std::initializer_list<const std::reference_wrapper<GL::Shader>> shaders){
	// attach compiled shaders
	for (GL::Shader& sh : shaders){
		glAttachShader(program_id, sh.id);
	}

	glLinkProgram(program_id);

	// detach shaders for later cleanup
	for (GL::Shader& sh : shaders){
		glDetachShader(program_id, sh.id);
	}
}

void Program::link_TF(const size_t n, const char** fb_varyings, std::initializer_list<const std::reference_wrapper<GL::Shader>> shaders){
	// link with transform feedback varyings
	// set TF varyings
	glTransformFeedbackVaryings(program_id, n, fb_varyings, GL_INTERLEAVED_ATTRIBS);
	//std::cout << glGetError() << std::endl;

	link(shaders);
}

GLint Program::get_uniform(const char* name) const {
	return glGetUniformLocation(program_id, name);
}

GLint Program::get_attrib(const char* name) const {
	return glGetAttribLocation(program_id, name);
}

void init_bar_shader(Program& sh_bars, const bool rainbow){
	const char* vertex_shader = 
	#include "shader/bar.vert"
	;
	GL::Shader vs(vertex_shader, GL_VERTEX_SHADER);

	// fragment shader
	const char* fragment_shader;
	if(rainbow){
		fragment_shader =
		#include "shader/rainbow.frag"
		;
	}else{
		fragment_shader =
		#include "shader/simple.frag"
		;
	}
	GL::Shader fs(fragment_shader, GL_FRAGMENT_SHADER);
	
	// geometry shader
	// draw bars
	const char* geometry_shader = 
	#include "shader/bar.geom"
	;
	GL::Shader gs(geometry_shader, GL_GEOMETRY_SHADER);

	// link shaders
	sh_bars.link({fs, vs, gs});
}

void init_line_shader(Program& sh_lines){
	// fragment shader
	const char* fragment_shader = 
	#include "shader/simple.frag"
	;
	GL::Shader fs(fragment_shader, GL_FRAGMENT_SHADER);
	
	const char* vs_lines_code = 
	#include "shader/lines.vert"
	;
	GL::Shader vs_lines(vs_lines_code, GL_VERTEX_SHADER);
	
	sh_lines.link({fs, vs_lines});
}

void init_bar_gravity_shader(Program& sh_bar_gravity){
	const char* vertex_shader = 
	#include "shader/bar_pre.vert"
	;
	GL::Shader vs(vertex_shader, GL_VERTEX_SHADER);

	const char* varyings[3] = {"v_gravity", "v_time", "v_y"};
	sh_bar_gravity.link_TF(3, varyings, {vs});
}
