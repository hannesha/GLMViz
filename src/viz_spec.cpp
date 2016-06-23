// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <math.h>

// Include FFTW3
#include <fftw3.h>

// Include basic GL utility headers
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "iostream"
#include "Shader.cpp"
#include "Program.cpp"
#include "FFT.cpp"
#include "Input.cpp"

const int AA_Samples = 4;
const int WIN_Height = 1024;
const int WIN_Width = 768;

const uint16_t fps = 60;
const uint16_t duration = 100; //(ms)
const uint16_t FS = 44100;
const size_t n_samples = FS * duration / 1000;
const size_t fft_size = 2<<13; //2^14

float top_color[] = {0.827/2.0, 0.149/2.0, 0.008/2.0, 1.0};
float bot_color[] = {0.275/2.0, 0.282/2.0, 0.294/2.0, 1.0};
float line_color[] = {0.275, 0.282, 0.294, 1.0};

//std::vector<int16_t> v_data(n_samples);

void init_x_data(const size_t);
void update_y_buffer(FFT&);
void update_x_buffer();
void init_buffers(Program&, FFT&);
void read_fifo(const int);

/*void init_fft();
void compute_fft();
void destroy_fft();
void calculate_window(const size_t);*/

size_t fft_output_size = fft_size/2+1;
/*double *fft_input;
std::vector<double> window;
fftw_complex *fft_output;
fftw_plan fft_plan;*/
const float d_freq = (float) FS / (float) fft_output_size;
const float fft_scale = 1.0f/((float)(n_samples/2+1)*32768.0f);
const float slope = 0.5f;
const float offset = 1.0f;

glm::mat4 view = glm::lookAt(
    glm::vec3(0.8f, 0.0f, 2.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
); 
glm::mat4 projection = glm::perspective(glm::radians(45.0f),  (float)WIN_Height / (float)WIN_Width, 1.0f, 10.0f);
glm::mat4 transformation = glm::ortho(-1.0,1.0,-1.0,1.0,-1.0,1.0);

void set_transformation(Program&);

// buffers and the corresponding vectors
GLuint vao_spec;

const size_t output_size = 100; 
GLuint y_buffer;
GLuint x_buffer;
std::vector<float> x_data(output_size);

// buffers and shaders for dB lines
GLuint vao_db;

const float lines[]  = {
	-1.0,  0.0, 1.0,  0.0, //   0dB
	-1.0, -0.5, 1.0, -0.5, // -10dB
	-1.0, -1.0, 1.0, -1.0, // -20dB
	-1.0, -1.5, 1.0, -1.5, // -30dB
	-1.0, -2.0, 1.0, -2.0, // -40dB
	-1.0, -2.5, 1.0, -2.5, // -50dB
	-1.0, -3.0, 1.0, -3.0, // -60dB
	-1.0, -3.5, 1.0, -3.5, // -70dB
	-1.0, -4.0, 1.0, -4.0  // -80dB
};
GLuint line_buffer;
void init_lines(Program&);

// handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
} 

int main(){
	
	// init GLFW
	if(!glfwInit()){
		fprintf(stderr, "GLFW init failed!\n");
		return -1;
	}
	
	glfwWindowHint(GLFW_SAMPLES, AA_Samples);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	std::stringstream title;
	title << "Spectrum (fmax=" << output_size * d_freq << "Hz)" ;
	
	GLFWwindow* window;
	window = glfwCreateWindow( WIN_Height, WIN_Width, title.str().c_str(), NULL, NULL);
	if( window == NULL ){
		fprintf(stderr, "GLFW Window init failed!\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window); // init GLEW
	glewExperimental = true;

	if(glewInit() != GLEW_OK){
		fprintf(stderr, "GLEW init failed!\n");
		return -1;	
	}
	
	Program sh_spec;
	Program sh_db;
	init_shaders(sh_spec, sh_db);
	
	FFT fft(fft_size);
	std::vector<int16_t> v_data(n_samples);
	
	init_x_data(output_size);
	//init_fft();
	init_buffers(sh_spec, fft);
	init_lines(sh_db);
	
	set_transformation(sh_spec);
	set_transformation(sh_db);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	Input fifo("/tmp/mpd.fifo");
	
	if (fifo.is_open()){
		// init read thread
		std::thread th_read;	
			
		// handle resizing	
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);	
		do{
			update_y_buffer(fft);

			//clear and draw		
			glClear(GL_COLOR_BUFFER_BIT);
			
			// render Lines
			sh_db.use();
			glBindVertexArray(vao_db);
			glDrawArrays(GL_LINES, 0, sizeof(lines) / sizeof(float));

			// render bars
			sh_spec.use();	
			glBindVertexArray(vao_spec);
			glDrawArrays(GL_POINTS, 0, output_size);	
			
			// Swap buffers
			glfwSwapBuffers(window);
			glfwPollEvents();
			
			// start read thread	
			th_read = std::thread([&]{ fifo.read_fifo(v_data, fft); });
			
			usleep(1000000 / fps);

			th_read.join();
		} // Wait until window is closed
		while(glfwWindowShouldClose(window) == 0);
	}	
	// clear buffers
	glDeleteBuffers(1, &y_buffer);
	glDeleteBuffers(1, &x_buffer);
	glDeleteBuffers(1, &line_buffer);
	glDeleteVertexArrays(1, &vao_db);
	glDeleteVertexArrays(1, &vao_spec);
	

	glfwTerminate();

	return 0;
}

// update y_data buffer
void update_y_buffer(FFT& fft){
	glBindBuffer(GL_ARRAY_BUFFER, y_buffer);
	glBufferData(GL_ARRAY_BUFFER, output_size * sizeof(fftw_complex), fft.output, GL_DYNAMIC_DRAW);
}

void update_x_buffer(){
	glBindBuffer(GL_ARRAY_BUFFER, x_buffer);
	glBufferData(GL_ARRAY_BUFFER, x_data.size() * sizeof(float), &x_data[0], GL_DYNAMIC_DRAW);
}

void init_buffers(Program& sh_spec, FFT& fft){
	// generate spectrum vao
	glGenVertexArrays(1, &vao_spec);

	glBindVertexArray(vao_spec);
	// Generate the y_position buffer
	glGenBuffers(1, &y_buffer);
	// bind the y_buffer for modification
	glBindBuffer(GL_ARRAY_BUFFER, y_buffer);
	// fill the y_buffer
	glBufferData(GL_ARRAY_BUFFER, output_size * sizeof(fftw_complex), fft.output, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	
	/* X position buffer */
	glGenBuffers(1, &x_buffer);

	glBindBuffer(GL_ARRAY_BUFFER, x_buffer);
	glBufferData(GL_ARRAY_BUFFER, x_data.size() * sizeof(float), &x_data[0], GL_DYNAMIC_DRAW);

	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	
	sh_spec.use();
	// init the shader uniform values
	GLint i_fft_scale = sh_spec.get_uniform("fft_scale");
	glUniform1f(i_fft_scale, fft_scale);	

	GLint i_slope = sh_spec.get_uniform("slope");
	glUniform1f(i_slope, slope);
	
	GLint i_offset = sh_spec.get_uniform("offset");
	glUniform1f(i_offset, offset);

	GLint i_width = sh_spec.get_uniform("width");
	glUniform1f(i_width, 1.5f/(float)output_size);
	
	// set bar color gradients	
	GLint i_top_color = sh_spec.get_uniform("top_color");
	glUniform4fv(i_top_color, 1, top_color);

	GLint i_bot_color = sh_spec.get_uniform("bot_color");
	glUniform4fv(i_bot_color, 1, bot_color);	
}

void init_lines(Program& sh_db){
	glGenVertexArrays(1, &vao_db);
	
	glBindVertexArray(vao_db);
	
	glGenBuffers(1, &line_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	sh_db.use();
	
	GLint i_slope = sh_db.get_uniform("slope");
	glUniform1f(i_slope, slope);
	
	GLint i_offset = sh_db.get_uniform("offset");
	glUniform1f(i_offset, offset);

	GLint i_line_color = sh_db.get_uniform("line_color");
	glUniform4fv(i_line_color, 1, line_color);
}

void set_transformation(Program& sh){
	sh.use();
	
	GLint i_trans = sh.get_uniform("trans");
	glUniformMatrix4fv(i_trans, 1, GL_FALSE, glm::value_ptr(transformation));
}

void init_x_data(const size_t size){
	x_data.resize(size);
	for(unsigned int i = 0; i < size; i++){
		x_data[i] = (((float) i + 0.5) - ((float)size * 0.5)) / ((float)size * 0.5);
	}
}

/*void init_fft(){
	// allocate memory for fft input/output
	fft_input = (double*) (fftw_malloc(sizeof(double) * fft_size));
	fft_output = (fftw_complex*) (fftw_malloc(sizeof(fftw_complex) * fft_output_size));
	fft_plan = fftw_plan_dft_r2c_1d(fft_size, fft_input, fft_output, FFTW_ESTIMATE);
}

void destroy_fft(){
	// free fft input/output and delete fft plan
	fftw_destroy_plan(fft_plan);
	fftw_free(fft_input);
	fftw_free(fft_output);
}

void compute_fft(){
	// apply window function and pad data
	size_t window_size = fft_size < n_samples ? fft_size : n_samples;
	
	// recalculate the window if the data size changes
	if (window.size() != window_size){
		calculate_window(window_size);
	}
	
	for(unsigned int i = 0; i < fft_size; i++){
		// apply hann window with corrected factors (a * 2)
		fft_input[i] = i < n_samples ? (float)v_data[i] * window[i] : 0;
	}	
	// calculate fft
	fftw_execute(fft_plan);
}

void calculate_window(const size_t size){
	window.resize(size);
	double N_1 = 1.0 / (double)(size-1);
	for(unsigned int i = 0; i < size; i++){
		window[i] = 1.0 - cos(2.0*M_PI*(double)i * N_1);
	}
}*/
