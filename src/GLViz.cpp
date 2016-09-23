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

#include "GLViz.hpp"

#include <memory>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void init_x_data(std::vector<float>&, const size_t);
void update_y_buffer(FFT&, Config&);
void update_x_buffer(std::vector<float>&);
void init_buffers(Program&, Config&);
void init_bars_pre(Program&, Config&);

void init_lines(Program&, Config&);

glm::mat4 transformation = glm::ortho(-1.0,1.0,-1.0,1.0,-1.0,1.0);

void set_transformation(Program&);

// buffers and the corresponding vectors
GLuint vao_spec_pre;

GLuint vao_spec;

//const size_t output_size = 100; 
GLuint y_buffer;
GLuint x_buffer;
// transform feedback buffers for gravity
GLuint fb1, fb2;

// buffers and shaders for dB lines
GLuint vao_db;

GLuint line_buffer;

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

// handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
} 

void dumpBuffer(Buffer& buffer){
	auto lock = buffer.lock();
	std::cout << buffer.v_buffer[0] << " " << buffer.v_buffer[1] << " " << buffer.v_buffer[2] << " " << buffer.v_buffer[3] << std::endl;
}

int main(){
	
	// init GLFW
	if(!glfwInit()){
		fprintf(stderr, "GLFW init failed!\n");
		return -1;
	}
	
	Config config;
		
	glfwWindowHint(GLFW_SAMPLES, config.w_aa);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	std::stringstream title;
	title << "Spectrum (fmax=" << config.output_size * config.d_freq << "Hz)" ;
	
	GLFWwindow* window;
	window = glfwCreateWindow( config.w_height, config.w_width, title.str().c_str(), NULL, NULL);
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
	Program sh_spec_pre;
	Program sh_db;
	init_bar_shader(sh_spec, config);
	init_line_shader(sh_db);
	init_bar_gravity_shader(sh_spec_pre);
	
	FFT fft(config.fft_size);

	std::vector<float> x_data(config.output_size);

	init_x_data(x_data, config.output_size);
	init_buffers(sh_spec, config);
	init_bars_pre(sh_spec_pre, config);

	update_x_buffer(x_data);

	init_lines(sh_db, config);
	
	set_transformation(sh_spec);
	set_transformation(sh_db);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//	glEnable(GL_FRAMEBUFFER_SRGB);

	std::unique_ptr<Input> input;
	
	GLint arg_y = sh_spec.get_attrib("y");
	GLint arg_gravity_old = sh_spec_pre.get_attrib("gravity_old");
	GLint arg_time_old = sh_spec_pre.get_attrib("time_old");

	switch (config.source){
#ifdef WITH_PULSE
	case Source::PULSE:
		input = make_unique<Pulse>(Pulse::get_default_sink(), 44);
		break;
#endif	
	default:
		input = make_unique<Fifo>(config.fifo_file, 44);
	}

	if (input->is_open()){
		Buffer buffer(config.buf_size);

		bool p_running = true;
		std::thread th_input = std::thread([&]{
			while(p_running){
				input->read(buffer);
			}
		});

		// handle resizing	
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);	
		do{
			std::thread th_fps = std::thread([&]{usleep(1000000 / config.fps);});

			//dumpBuffer(buffer);
			fft.calculate(buffer);
			update_y_buffer(fft, config);
			
			//clear and draw		
			glClear(GL_COLOR_BUFFER_BIT);
			
			// render Lines
			sh_db.use();
			glBindVertexArray(vao_db);
			glDrawArrays(GL_LINES, 0, sizeof(lines) / sizeof(float));
			
		
			glBindVertexArray(vao_spec_pre);
			glBindBuffer(GL_ARRAY_BUFFER, fb2);			
			glVertexAttribPointer(arg_gravity_old, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
			glVertexAttribPointer(arg_time_old, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)(sizeof(float)));

			sh_spec_pre.use();	
			// bind fist feedback buffer as TF buffer
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, fb1);
			glBeginTransformFeedback(GL_POINTS);
			
			glEnable(GL_RASTERIZER_DISCARD);
			glDrawArrays(GL_POINTS, 0, config.output_size);
			glDisable(GL_RASTERIZER_DISCARD);
			// disable TF
			glEndTransformFeedback();	

			//undbind both feedback buffers and swap them
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			std::swap(fb1,fb2);
				
			// render bars
			sh_spec.use();	
			glBindVertexArray(vao_spec);
			// use second feedback buffer for drawing
			glBindBuffer(GL_ARRAY_BUFFER, fb2);
			glVertexAttribPointer(arg_y, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)(2*sizeof(float)));	
			glDrawArrays(GL_POINTS, 0, config.output_size);

			// Swap buffers
			glfwSwapBuffers(window);
			glfwPollEvents();
			
			//p.read(v_data);
			//fft.calculate(v_data);		
			th_fps.join();
		} // Wait until window is closed
		while(glfwWindowShouldClose(window) == 0);
	
		p_running = false;
		th_input.join();
	}else{
		std::cerr << "Can't open audio source:" << config.fifo_file << std::endl;
	}

	// clear buffers
	glDeleteBuffers(1, &y_buffer);
	glDeleteBuffers(1, &x_buffer);
	glDeleteBuffers(1, &line_buffer);
	glDeleteBuffers(1, &fb1);
	glDeleteBuffers(1, &fb2);
	glDeleteVertexArrays(1, &vao_db);
	glDeleteVertexArrays(1, &vao_spec);
	glDeleteVertexArrays(1, &vao_spec_pre);

	glfwTerminate();

	return 0;
}

// update y_data buffer
void update_y_buffer(FFT& fft, Config &cfg){
	glBindBuffer(GL_ARRAY_BUFFER, y_buffer);
	glBufferData(GL_ARRAY_BUFFER, cfg.output_size * sizeof(fftw_complex), fft.output, GL_DYNAMIC_DRAW);
}

void update_x_buffer(std::vector<float>& data){
	glBindBuffer(GL_ARRAY_BUFFER, x_buffer);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_DYNAMIC_DRAW);
}

void init_buffers(Program& sh_spec, Config &cfg){
	// generate spectrum VAOs
	glGenVertexArrays(1, &vao_spec);

	// init ping-pong feedback buffers
	glGenBuffers(1, &fb1);
	glGenBuffers(1, &fb2);
	glBindBuffer(GL_ARRAY_BUFFER, fb1);
	glBufferData(GL_ARRAY_BUFFER, cfg.output_size * 3 *sizeof(float), 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, fb2);
	glBufferData(GL_ARRAY_BUFFER, cfg.output_size * 3 *sizeof(float), 0, GL_DYNAMIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, fb2);
	
	/* Post processing shader */
	glBindVertexArray(vao_spec);
	// X position buffer
	glGenBuffers(1, &x_buffer);

	glBindBuffer(GL_ARRAY_BUFFER, x_buffer);
	glBufferData(GL_ARRAY_BUFFER, cfg.output_size * sizeof(float), 0, GL_DYNAMIC_DRAW);

	GLint arg_x_data = sh_spec.get_attrib("x");
	glVertexAttribPointer(arg_x_data, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_x_data);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, fb2);
	GLint arg_y = sh_spec.get_attrib("y");
	// use third float of TF 
	//glVertexAttribPointer(arg_y, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)(2*sizeof(float)));
	glEnableVertexAttribArray(arg_y);	

	sh_spec.use();
	// Post compute specific uniforms
	GLint i_width = sh_spec.get_uniform("width");
	glUniform1f(i_width, cfg.bar_width/(float)cfg.output_size);
	
	// set bar color gradients	
	GLint i_top_color = sh_spec.get_uniform("top_color");
	glUniform4fv(i_top_color, 1, cfg.top_color);

	GLint i_bot_color = sh_spec.get_uniform("bot_color");
	glUniform4fv(i_bot_color, 1, cfg.bot_color);
	
	GLint i_gradient = sh_spec.get_uniform("gradient");
	glUniform1f(i_gradient, cfg.gradient);
}

void init_bars_pre(Program& sh_bars_pre, Config& cfg){
	glGenVertexArrays(1, &vao_spec_pre);

	/* Pre compute shader */
	glBindVertexArray(vao_spec_pre);
	
	// generate fft_data buffer
	glGenBuffers(1, &y_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, y_buffer);
	glBufferData(GL_ARRAY_BUFFER, cfg.output_size * sizeof(fftw_complex), 0, GL_DYNAMIC_DRAW);
	// set fft_data buffer as vec2 input for the shader
	GLint arg_fft_output = sh_bars_pre.get_attrib("a");
	glVertexAttribPointer(arg_fft_output, 2, GL_DOUBLE, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_fft_output);
	
	GLint arg_gravity_old = sh_bars_pre.get_attrib("gravity_old");
	// use first float of TF
	//glVertexAttribPointer(arg_gravity_old, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
	glEnableVertexAttribArray(arg_gravity_old);

	GLint arg_time_old = sh_bars_pre.get_attrib("time_old");
	// use second float of TF
	//glVertexAttribPointer(arg_time_old, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)(sizeof(float)));
	glEnableVertexAttribArray(arg_time_old);

	sh_bars_pre.use();
	// set Pre compute shader uniforms
	GLint i_fft_scale = sh_bars_pre.get_uniform("fft_scale");
	glUniform1f(i_fft_scale, cfg.fft_scale);	

	GLint i_slope = sh_bars_pre.get_uniform("slope");
	glUniform1f(i_slope, cfg.slope * 0.5);
	
	GLint i_offset = sh_bars_pre.get_uniform("offset");
	glUniform1f(i_offset, cfg.offset * 0.5);
	
	GLint i_gravity = sh_bars_pre.get_uniform("gravity");
	glUniform1f(i_gravity, cfg.gravity / cfg.fps);	
}

void init_lines(Program& sh_lines, Config &cfg){
	glGenVertexArrays(1, &vao_db);
	
	glBindVertexArray(vao_db);
	
	glGenBuffers(1, &line_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_DYNAMIC_DRAW);
	
	GLint arg_line_vert = sh_lines.get_attrib("pos");
	glVertexAttribPointer(arg_line_vert, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_line_vert);

	sh_lines.use();
	
	GLint i_slope = sh_lines.get_uniform("slope");
	glUniform1f(i_slope, cfg.slope);
	
	GLint i_offset = sh_lines.get_uniform("offset");
	glUniform1f(i_offset, cfg.offset);

	GLint i_line_color = sh_lines.get_uniform("line_color");
	glUniform4fv(i_line_color, 1, cfg.line_color);
}

void set_transformation(Program& sh){
	sh.use();
	
	GLint i_trans = sh.get_uniform("trans");
	glUniformMatrix4fv(i_trans, 1, GL_FALSE, glm::value_ptr(transformation));
}

void init_x_data(std::vector<float>& vec, const size_t size){
	vec.resize(size);
	for(unsigned int i = 0; i < size; i++){
		vec[i] = (((float) i + 0.5) - ((float)size * 0.5)) / ((float)size * 0.5);
	}
}
