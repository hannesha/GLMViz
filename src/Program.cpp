class Program {
	public:
		Program();
		~Program();
		void attach_shader(const Shader& s);
		void link();
		void link_TF(const size_t, const char**);
		void use();
		GLuint get_id() const;
		GLint get_uniform(const char*) const;
		GLint get_attrib(const char*) const;
	private:
		GLuint program_id;
		std::vector<GLuint> shaders;
};
void init_shaders(Program&, Program&);
void init_line_shader(Program&);
void init_bar_gravity_shader(Program&);

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

void Program::use(){
	glUseProgram(program_id);
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

void init_shaders(Program& sh_bars, Program& sh_lines){
	const char* vertex_shader = 
	"#version 150\n"
	"in float x;"
	"in float y;"
	"void main () {"
	"  gl_Position = vec4(x, y, 0.0, 1.0);"
	"}";
	Shader vs(vertex_shader, GL_VERTEX_SHADER);	


	// fragment shader
	const char* fragment_shader = 
	"#version 150\n"
	"in vec4 color;"
	"out vec4 f_color;"
	"void main () {"
	"  f_color = color;"
	"}";
	Shader fs(fragment_shader, GL_FRAGMENT_SHADER);
	
	// geometry shader
	// draw bars
	const char* geometry_shader = 
	"#version 150\n"
	"layout (points) in;"
	"layout (triangle_strip, max_vertices = 4) out;"
	"out vec4 color;"
	"uniform float width;"
	"uniform vec4 bot_color;"
	"uniform vec4 top_color;"
	"uniform mat4 trans;"
	"void main () {"
	"  float width_2 = width * 0.5;"
	"  float x1 = gl_in[0].gl_Position.x - width_2;"
	"  float x2 = gl_in[0].gl_Position.x + width_2;"
	"  color = bot_color;"
	"  gl_Position = trans * vec4(x1, -1.0, 0.0, 1.0);"
	"  EmitVertex();"
	"  color = top_color;"
	"  gl_Position = trans * (gl_in[0].gl_Position - vec4(width_2, 0.0, 0.0, 0.0));"
	"  EmitVertex();"
	"  color = bot_color;"
	"  gl_Position = trans * vec4(x2, -1.0, 0.0, 1.0);"	
	"  EmitVertex();"
	"  color = top_color;"
	"  gl_Position = trans * (gl_in[0].gl_Position + vec4(width_2, 0.0, 0.0, 0.0));"
	"  EmitVertex();"
	"  EndPrimitive();"
	"}";	
	Shader gs(geometry_shader, GL_GEOMETRY_SHADER);

	// link shaders
	sh_bars.attach_shader(fs);
	sh_bars.attach_shader(vs);
	sh_bars.attach_shader(gs);
	
	sh_bars.link();
	
	init_line_shader(sh_lines);
}

void init_line_shader(Program& sh_lines){
	// fragment shader
	const char* fragment_shader = 
	"#version 150\n"
	"in vec4 color;"
	"out vec4 f_color;"
	"void main () {"
	"  f_color = color;"
	"}";
	Shader fs(fragment_shader, GL_FRAGMENT_SHADER);
	
	const char* vs_lines_code = 
	"#version 150\n"
	"in vec2 pos;"
	"out vec4 color;"
	"uniform float slope;"
	"uniform float offset;"
	"uniform vec4 line_color;"
	"uniform mat4 trans;"
	"void main () {"
	"  color = line_color;"
	"  gl_Position = trans * vec4 (pos.x, clamp(slope * pos.y + offset, -1.0, 1.0) , 0.0, 1.0);"
	"}";
	Shader vs_lines(vs_lines_code, GL_VERTEX_SHADER);
	
	sh_lines.attach_shader(fs);
	sh_lines.attach_shader(vs_lines);
	sh_lines.link();
}

void init_bar_gravity_shader(Program& sh_bar_gravity){
	const char* vertex_shader = 
	"#version 150\n"
	"in vec2 a;"
	"in float gravity_old;"
	"in float time_old;"
	"out float v_gravity;"
	"out float v_time;"
	"out float v_y;"
	"uniform float fft_scale;"
	"uniform float slope;"
	"uniform float offset;"
	"uniform float gravity;"
	"const float lg = 1 / log(10);"
	"void main () {"
	"  float a_norm = length(a) * fft_scale;"
	"  float y = slope * log(a_norm) * lg + offset;"
	"  float yg = gravity_old - gravity * time_old * time_old;"
	"  if(y > yg){"
	"    v_gravity = y;"
	"    v_time = 0.0;"
	"  }else{"
	"    v_gravity = gravity_old;"
	"    v_time = time_old + 1.0;"
	"    y = yg;"
	"  }"
	"  v_y = y;"
	"}";
	Shader vs(vertex_shader, GL_VERTEX_SHADER);	
	
	sh_bar_gravity.attach_shader(vs);
	
	const char* varyings[3] = {"v_gravity", "v_time", "v_y"};
	sh_bar_gravity.link_TF(3, varyings);	
}
