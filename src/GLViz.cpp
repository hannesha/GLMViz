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

// buffer init and update functions
void update_b_fft(GL::Buffer&, FFT&, Config&);
void update_x_buffer(GL::Buffer&, Config&);
void init_bars(GL::VAO&, GL::Buffer&, Program&, Config&);
void init_bars_pre(GL::VAO&, GL::Buffer&, GL::Buffer&, GL::Buffer&, Program&, Config&);
void init_lines(GL::VAO&, GL::Buffer&, Program&, Config&);

glm::mat4 transformation = glm::ortho(-1.0,1.0,-1.0,1.0,-1.0,1.0);

void set_transformation(Program&);

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
	Program sh_lines;
	init_bar_shader(sh_spec, config);
	init_line_shader(sh_lines);
	init_bar_gravity_shader(sh_spec_pre);
	

	// create and initialize bar buffers
	GL::VAO v_bars;
	GL::Buffer b_x;
	update_x_buffer(b_x, config);
	init_bars(v_bars, b_x, sh_spec, config);

	// create and initialize precompute buffers
	GL::VAO v_bars_pre;
	GL::Buffer b_fft, b_fb1, b_fb2;
	init_bars_pre(v_bars_pre, b_fft, b_fb1, b_fb2, sh_spec_pre, config);

	// initialize dB lines
	GL::VAO v_lines;
	GL::Buffer b_lines;
	init_lines(v_lines, b_lines, sh_lines, config);
	
	set_transformation(sh_spec);
	set_transformation(sh_lines);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	std::unique_ptr<Input> input;

	// get TF specific attribute locations	
	GLint arg_y = sh_spec.get_attrib("y");
	GLint arg_gravity_old = sh_spec_pre.get_attrib("gravity_old");
	GLint arg_time_old = sh_spec_pre.get_attrib("time_old");

	// audio source configuration
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
		FFT fft(config.fft_size);

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

			// apply fft and update fft buffer
			fft.calculate(buffer);
			update_b_fft(b_fft, fft, config);
					
			glClear(GL_COLOR_BUFFER_BIT);
			
			// render Lines
			sh_lines.use();
			v_lines.bind();
			glDrawArrays(GL_LINES, 0, sizeof(lines) / sizeof(float));
			
			// gravity processing shader
			v_bars_pre.bind();
			b_fb2.bind();
			// set TF attribute pointers
			glVertexAttribPointer(arg_gravity_old, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
			glVertexAttribPointer(arg_time_old, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)(sizeof(float)));

			sh_spec_pre.use();	
			// bind fist feedback buffer as TF buffer
			b_fb1.tfbind();
			glBeginTransformFeedback(GL_POINTS);
			
			glEnable(GL_RASTERIZER_DISCARD);
			glDrawArrays(GL_POINTS, 0, config.output_size);
			glDisable(GL_RASTERIZER_DISCARD);
			// disable TF
			glEndTransformFeedback();	

			//undbind both feedback buffers and swap them
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
			std::swap(b_fb1.id, b_fb2.id);
			

			// render bars
			sh_spec.use();	
			v_bars.bind();
			// use second feedback buffer for drawing
			b_fb2.bind();
			// reconfigure attribute pointer
			glVertexAttribPointer(arg_y, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)(2*sizeof(float)));	
			glDrawArrays(GL_POINTS, 0, config.output_size);

			// Swap buffers
			glfwSwapBuffers(window);
			glfwPollEvents();
			
			th_fps.join();
		} // Wait until window is closed
		while(glfwWindowShouldClose(window) == 0);

		// stop Input thread	
		p_running = false;
		th_input.join();
	}else{
		std::cerr << "Can't open audio source:" << config.fifo_file << std::endl;
	}

	glfwTerminate();
	return 0;
}

void update_b_fft(GL::Buffer& b_fft, FFT& fft, Config& cfg){
	//glBindBuffer(GL_ARRAY_BUFFER, y_buffer);
	b_fft.bind();
	glBufferData(GL_ARRAY_BUFFER, cfg.output_size * sizeof(fftw_complex), fft.output, GL_DYNAMIC_DRAW);
}

void update_x_buffer(GL::Buffer& b_x, Config& cfg){
	std::vector<float> x_data(cfg.output_size);
	float size = (float)cfg.output_size;
	
	// generate x positions ranging from -1 to 1
	for(unsigned int i = 0; i < size; i++){
		x_data[i] = (((float) i + 0.5) - (size * 0.5)) / (size * 0.5);
	}

	b_x.bind();
	glBufferData(GL_ARRAY_BUFFER, x_data.size() * sizeof(float), &x_data[0], GL_STATIC_DRAW);
}

void init_bars(GL::VAO& v_bars, GL::Buffer& b_x, Program& sh_spec, Config& cfg){
	v_bars.bind();

	b_x.bind();
	// set x position data in bar VAO
	GLint arg_x_data = sh_spec.get_attrib("x");
	glVertexAttribPointer(arg_x_data, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_x_data);
	
	// enable the preprocessed y attribute
	GLint arg_y = sh_spec.get_attrib("y");
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

void init_bars_pre(GL::VAO& v_bars_pre, GL::Buffer& b_fft, GL::Buffer& b_fb1, GL::Buffer& b_fb2, Program& sh_bars_pre, Config& cfg){
	/* Pre compute shader */
	v_bars_pre.bind();
	
	// init ping-pong feedback buffers
	b_fb1.bind();
	glBufferData(GL_ARRAY_BUFFER, cfg.output_size * 3 * sizeof(float), 0, GL_DYNAMIC_DRAW);

	b_fb2.bind();
	glBufferData(GL_ARRAY_BUFFER, cfg.output_size * 3 * sizeof(float), 0, GL_DYNAMIC_DRAW);

	b_fft.bind();
	// set fft_data buffer as vec2 input for the shader
	GLint arg_fft_output = sh_bars_pre.get_attrib("a");
	glVertexAttribPointer(arg_fft_output, 2, GL_DOUBLE, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_fft_output);

	// enable precompute shader attributes	
	GLint arg_gravity_old = sh_bars_pre.get_attrib("gravity_old");
	glEnableVertexAttribArray(arg_gravity_old);

	GLint arg_time_old = sh_bars_pre.get_attrib("time_old");
	glEnableVertexAttribArray(arg_time_old);


	sh_bars_pre.use();
	// set precompute shader uniforms
	GLint i_fft_scale = sh_bars_pre.get_uniform("fft_scale");
	glUniform1f(i_fft_scale, cfg.fft_scale);	

	GLint i_slope = sh_bars_pre.get_uniform("slope");
	glUniform1f(i_slope, cfg.slope * 0.5);
	
	GLint i_offset = sh_bars_pre.get_uniform("offset");
	glUniform1f(i_offset, cfg.offset * 0.5);
	
	GLint i_gravity = sh_bars_pre.get_uniform("gravity");
	glUniform1f(i_gravity, cfg.gravity / cfg.fps);	
}

void init_lines(GL::VAO& v_lines, GL::Buffer& b_lines, Program& sh_lines, Config& cfg){
	v_lines.bind();

	b_lines.bind();
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);
	
	GLint arg_line_vert = sh_lines.get_attrib("pos");
	glVertexAttribPointer(arg_line_vert, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_line_vert);


	sh_lines.use();
	// set dB line specific arguments
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
