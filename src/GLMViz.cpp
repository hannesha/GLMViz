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

typedef std::unique_ptr<Spectrum> Spec_ptr;
typedef std::unique_ptr<Oscilloscope> Osc_ptr;

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

// input thread wrapper
class Input_thread{
	public:
		template <typename Funct> Input_thread(Funct f):
			running(true),
			th_input([&]{while(running){ f(); };})
			{ };

		~Input_thread(){ running = false; th_input.join(); };

	private:
		std::atomic<bool> running;
		std::thread th_input;
};

// config reload signal handler
static_assert(ATOMIC_BOOL_LOCK_FREE, "std::atomic<bool> isn't lock free!");
std::atomic<bool> config_reload (false);
void sighandler(int signal){
	config_reload = true;
}

// config reload key handler
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(key == GLFW_KEY_R && action == GLFW_PRESS){
		config_reload = true;
	}
}

// handle window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

std::string generate_title(Config& config){
	std::stringstream title;
	title << "Spectrum (fmax=" << config.spec_default.output_size * config.d_freq << "Hz)";
	return title.str();
}

// mainloop template
template <typename Fupdate, typename Fdraw>
void mainloop(Config& config, GLFWwindow* window, Fupdate f_update, Fdraw f_draw){
	do{
		if(config_reload){
			std::cout << "reloading config" << std::endl;
			config_reload = false;
			config.reload();

			// generate new title
			std::string title = generate_title(config);
			glfwSetWindowTitle(window, title.c_str());

			// update uniforms, resize buffers
			f_update();
		}

		std::chrono::time_point<std::chrono::steady_clock> t_fps = std::chrono::steady_clock::now() + std::chrono::microseconds(1000000 / config.fps -100);

		glClear(GL_COLOR_BUFFER_BIT);

		// draw
		f_draw();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// wait for fps timer
		std::this_thread::sleep_until(t_fps);
	}
	while(glfwWindowShouldClose(window) == 0);
}

// defines how many samples are read from the buffer during each loop iteration
// this number has to be even in stereo mode
#define SAMPLES 220

int main(){
	try{
		// read config
		Config config;

		Input::Ptr input;

		// audio source configuration
		switch (config.source){
#ifdef WITH_PULSE
		case Source::PULSE:
			input = make_unique<Pulse>(Pulse::get_default_sink(), config.FS, SAMPLES, config.stereo);
			break;
#endif
		default:
			input = make_unique<Fifo>(config.fifo_file, SAMPLES);
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
		window = glfwCreateWindow(config.w_height, config.w_width, title.c_str(), NULL, NULL);
		if( window == NULL ){
			throw glfw_error("Failed to create GLFW Window!");
		}

		glfwMakeContextCurrent(window); 

		// set clear color to black
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glEnable(GL_BLEND);
		// handle resizing
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

		glfwSetKeyCallback(window, key_callback);

		std::vector<Spec_ptr> spectra;
		std::vector<Osc_ptr> oscilloscopes;

		for(unsigned i = 0; i < config.spectra.size(); i++){
			spectra.push_back(make_unique<Spectrum>(config, i));
		}

		for(unsigned i = 0; i < config.oscilloscopes.size(); i++){
			oscilloscopes.push_back(make_unique<Oscilloscope>(config, i));
		}

		if(!config.stereo){
			// create audio buffer
			Buffer<int16_t> buffer(config.buf_size);

			// start input thread
			Input_thread inth([&]{
				input->read(buffer);
			});

			// create fft
			FFT fft(config.fft_size);

			// mono mainloop
			mainloop(config, window,
			[&]{
				// resize buffers and reconfigure renderer
				buffer.resize(config.buf_size);

				for(Spec_ptr& s : spectra){
					s->configure(config);
				}

				for(Osc_ptr& o : oscilloscopes){
					o->configure(config);
				}
			},
			[&]{
				// update all locking renderer first
				fft.calculate(buffer);
				for(Osc_ptr& o : oscilloscopes){
					o->update_buffer(buffer);
				}
				// draw spectra and oscilloscopes
				for(Spec_ptr& s : spectra){
					s->update_fft(fft);
					s->draw();
				}
				for(Osc_ptr& o : oscilloscopes){
					o->draw();
				}
			});
		}else{
			// create left and right audio buffer
			Buffer<int16_t> lbuffer(config.buf_size);
			Buffer<int16_t> rbuffer(config.buf_size);

			// start stereo input thread
			Input_thread inth([&]{
				input->read_stereo(lbuffer, rbuffer);
			});

			// create shared FFT pointer vector
			std::vector<std::shared_ptr<FFT>> ffts;
			ffts.push_back(std::make_shared<FFT>(config.fft_size));
			ffts.push_back(std::make_shared<FFT>(config.fft_size));

			// stereo mainloop
			mainloop(config, window,
			[&]{
				// resize buffers
				lbuffer.resize(config.buf_size);
				rbuffer.resize(config.buf_size);
				// update spectrum/oscilloscope renderer
				for(Spec_ptr& s : spectra){
					s->configure(config);
				}
				for(Osc_ptr& o : oscilloscopes){
					o->configure(config);
				}
			},
			[&]{
				ffts[0]->calculate(lbuffer);
				ffts[1]->calculate(rbuffer);
				for(Osc_ptr& o : oscilloscopes){
					o->update_buffer(lbuffer, rbuffer);
				}

				for(Spec_ptr& s : spectra){
					s->update_fft(ffts);
					s->draw();
				}

				for(Osc_ptr& o : oscilloscopes){
					o->draw();
				}
			});
		}

	}catch(std::runtime_error& e){
		// print error message and terminate with error code 1
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
