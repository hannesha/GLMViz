#include "Program.hpp"

Program::Program(){
	program_id = glCreateProgram();
}

Program::~Program(){
	glDeleteProgram(program_id);
}

void Program::attach_shader(const Shader& s){
	glAttachShader(program_id, s.get_shader());
	shaders.push_back(s.get_shader());
}

void Program::link(){
	// link, cleanup
	glLinkProgram(program_id);
	//std::cout << glGetError() << std::endl;
	
	// detach all shaders
	for (GLuint sh : shaders){
		glDetachShader(program_id, sh);
	}
}

void Program::link_TF(const size_t n, const char** fb_varyings){
	// set TF varyings
	glTransformFeedbackVaryings(program_id, n, fb_varyings, GL_INTERLEAVED_ATTRIBS);	
	//std::cout << glGetError() << std::endl;

	link();
}

GLuint Program::get_id() const {
	return program_id;
}

GLint Program::get_uniform(const char* name) const {
	return glGetUniformLocation(program_id, name);
}

GLint Program::get_attrib(const char* name) const {
	return glGetAttribLocation(program_id, name);
}

void init_bar_shader(Program& sh_bars){
	const char* vertex_shader = 
	#include "shader/bar.vs"
	;
	Shader vs(vertex_shader, GL_VERTEX_SHADER);	


	// fragment shader
	const char* fragment_shader = 
	#include "shader/simple.fs"
//	#include "shader/rainbow.fs"
	;
	Shader fs(fragment_shader, GL_FRAGMENT_SHADER);
	
	// geometry shader
	// draw bars
	const char* geometry_shader = 
	#include "shader/bar.gs"
	;
	Shader gs(geometry_shader, GL_GEOMETRY_SHADER);

	// link shaders
	sh_bars.attach_shader(fs);
	sh_bars.attach_shader(vs);
	sh_bars.attach_shader(gs);
	
	sh_bars.link();
}

void init_line_shader(Program& sh_lines){
	// fragment shader
	const char* fragment_shader = 
	#include "shader/simple.fs"
	;
	Shader fs(fragment_shader, GL_FRAGMENT_SHADER);
	
	const char* vs_lines_code = 
	#include "shader/lines.vs"
	;
	Shader vs_lines(vs_lines_code, GL_VERTEX_SHADER);
	
	sh_lines.attach_shader(fs);
	sh_lines.attach_shader(vs_lines);
	sh_lines.link();
}

void init_bar_gravity_shader(Program& sh_bar_gravity){
	const char* vertex_shader = 
	#include "shader/bar_pre.vs"
	;
	Shader vs(vertex_shader, GL_VERTEX_SHADER);	
	
	sh_bar_gravity.attach_shader(vs);
	
	const char* varyings[3] = {"v_gravity", "v_time", "v_y"};
	sh_bar_gravity.link_TF(3, varyings);	
}
