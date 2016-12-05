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

#include "GLMViz.hpp"

#include <memory>
#include <stdexcept>
#include <chrono>
#include <csignal>
#include <atomic>

// make_unique template for backwards compatibility
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class glfw_error : public std::runtime_error {
public :
	glfw_error(const char* what_arg) : std::runtime_error(what_arg){};
};

// glfw raii wrapper
class GLFW{
	public:
		GLFW(){ if(!glfwInit()) throw std::runtime_error("GLFW init failed!"); };
		~GLFW(){ glfwTerminate(); };
};

// config reload signal handler
static_assert(ATOMIC_BOOL_LOCK_FREE, "std::atomic<bool> isn't lock free!");
std::atomic<bool> config_reload (false);
void sighandler(int signal){
	config_reload = true;
}

// handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

std::string generate_title(Config& config){
	std::stringstream title;
	title << "Spectrum (fmax=" << config.output_size * config.d_freq << "Hz)";
	return title.str();
}

int main(){
	try{
		// read config
		Config config;

		Input::Ptr input;

		// audio source configuration
		switch (config.source){
#ifdef WITH_PULSE
		case Source::PULSE:
			input = make_unique<Pulse>(Pulse::get_default_sink(), config.FS, 441);
			break;
#endif
		default:
			input = make_unique<Fifo>(config.fifo_file, 441);
		}

		// attach SIGUSR1 signal handler
		std::signal(SIGUSR1, sighandler);

		// init GLFW
		GLFW glfw;

		glfwWindowHint(GLFW_SAMPLES, config.w_aa);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		std::string title = generate_title(config);
		// create GLFW window
		GLFWwindow* window;
		window = glfwCreateWindow( config.w_height, config.w_width, title.c_str(), NULL, NULL);
		if( window == NULL ){
			throw glfw_error("Failed to create GLFW Window!");
		}

		glfwMakeContextCurrent(window); 

		// set clear color to black
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glEnable(GL_BLEND);

		// initialize spectrum renderer
		Spectrum spec(config);
		//Oscilloscope osc(config);

		// create audio buffer and FFT
		Buffer<int16_t> buffer(config.buf_size);
		FFT fft(config.fft_size);

		// start input thread
		std::atomic<bool> p_running (true);
		std::thread th_input = std::thread([&]{
			while(p_running){
				input->read(buffer);
			}
		});

		// handle resizing
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

		do{
			if(config_reload){
				std::cout << "reloading config" << std::endl;
				config_reload = false;
				config.reload();

				// resize buffer if necessary
				if(buffer.size != (size_t)config.buf_size) buffer.resize(config.buf_size);
				// update shader uniforms
				spec.set_uniforms(config);
				spec.resize(config.output_size);
				// update window title
				title = generate_title(config);
				glfwSetWindowTitle(window, title.c_str());
				//osc.set_uniforms(config);
				//osc.update_x_buffer(config.buf_size);
			}

			std::chrono::time_point<std::chrono::steady_clock> t_fps = std::chrono::steady_clock::now() + std::chrono::microseconds(1000000 / config.fps -100);

			// apply fft and update fft buffer
			fft.calculate(buffer);

			// update spectrum renderer buffer
			spec.update_fft(fft);
			//osc.update_buffer(buffer);

			glClear(GL_COLOR_BUFFER_BIT);

			spec.draw(config.draw_dB_lines);
			//osc.draw(config.buf_size);

			// Swap buffers
			glfwSwapBuffers(window);
			glfwPollEvents();

			// wait for fps timer
			std::this_thread::sleep_until(t_fps);
		} // Wait until window is closed
		while(glfwWindowShouldClose(window) == 0);

		// stop Input thread
		p_running = false;
		th_input.join();

	}catch(std::runtime_error& e){
		// print error message and terminate with error code 1
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
