class Shader {
	public:
		GLuint get_shader() const;
		Shader(const char* , GLuint);
		~Shader();
	private:
		GLuint shader;
};

Shader::Shader(const char* code, GLuint type){
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);
}

Shader::~Shader(){
	glDeleteShader(shader);
}

GLuint Shader::get_shader() const {
	return shader;
}
