/*
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
#include "Multisampler.hpp"

#include <memory>
#include <stdexcept>
#include <chrono>
#include <csignal>

// config reload signal handler
static_assert(ATOMIC_BOOL_LOCK_FREE, "std::atomic<bool> isn't lock free!");
std::atomic<bool> config_reload (false);
void sighandler(int signal){
	config_reload = true;
}

// set glClear color
void set_bg_color(const Module_Config::Color& color){
	glClearColor(color.rgba[0], color.rgba[1], color.rgba[2], color.rgba[3]);
}

std::string generate_title(const Config&);

// create or delete renderers to match the corresponding configs
template <typename R_vector, typename C_vector>
void update_render_configs(R_vector&, C_vector&);

// glfw specific code
#ifndef WITH_TRANSPARENCY
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

// glfw mainloop
template <typename Fupdate, typename Fdraw>
void mainloop(Config& config, GLFWwindow* window, Fupdate f_update, Fdraw f_draw){
	std::chrono::time_point<std::chrono::steady_clock> t_start;
	std::chrono::time_point<std::chrono::steady_clock> t_stop;
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

		t_start = std::chrono::steady_clock::now();

		glClear(GL_COLOR_BUFFER_BIT);

		// draw
		std::chrono::duration<float> dt = (t_start - t_stop);
		f_draw(dt.count());

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		// wait for fps timer
		std::this_thread::sleep_until(t_start + std::chrono::microseconds(1000000 / config.fps -100));
		t_stop = t_start;
	}
	while(glfwWindowShouldClose(window) == 0);
}
#else
// glx mainloop
bool closing = false;
template <typename Fupdate, typename Fdraw>
void mainloop(Config& config, GLXwindow& window, Fupdate f_update, Fdraw f_draw){
	Atom wm_delete_window = XInternAtom(window.display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(window.display, window.win, &wm_delete_window, 1);

	int width = config.w_width;
	int height = config.w_height;
	// create multisample framebuffer
	GL::Multisampler msaa(config.w_aa, width, height);
	glEnable(GL_MULTISAMPLE);

	std::chrono::time_point<std::chrono::steady_clock> t_start;
	std::chrono::time_point<std::chrono::steady_clock> t_stop;
	while(!closing){
		// handle X events
		while(XPending(window.display) > 0){
			XEvent event;
			XNextEvent(window.display, &event);

			switch(event.type){
			case ClientMessage:
				if((Atom)event.xclient.data.l[0] == wm_delete_window){
					closing = true;
				}
				break;

			case Expose:
				XWindowAttributes wattr;
				XGetWindowAttributes(window.display, window.win, &wattr);
				// resize viewport and multisample framebuffer
				if(width != wattr.width || height != wattr.height){
					width = wattr.width;
					height = wattr.height;

					glViewport(0, 0, width, height);
					msaa.resize(config.w_aa, width, height);
				}
				break;
			}
		}

		if(config_reload){
			std::cout << "reloading config" << std::endl;
			config_reload = false;
			config.reload();

			// generate new title
			std::string title = generate_title(config);
			window.set_title(title);

			// update uniforms, resize buffers
			f_update();
		}

		t_start = std::chrono::steady_clock::now();

		msaa.bind();
		glClear(GL_COLOR_BUFFER_BIT);

		// draw
		std::chrono::duration<float> dt = (t_start - t_stop);
		f_draw(dt.count());

		// Swap buffers
		msaa.blit(width, height);
		window.swapBuffers();
		//glfwPollEvents();

		// wait for fps timer
		std::this_thread::sleep_until(t_start + std::chrono::microseconds(1000000 / config.fps -100));
		t_stop = t_start;
	}
}
#endif

inline float normalize_rms(float, float, float);
Input::Ptr make_input(const Module_Config::Input&);

int main(int argc, char *argv[]){
	try{
		// construct config file path from first argument
		std::string config_file = "";
		if(argc > 1){
			config_file = argv[1];
		}
		// read config
		Config config(config_file);

		// create audio buffer
		std::vector<Buffer<int16_t>> buffers;
		buffers.emplace_back(config.buf_size);

		// create fft
		std::vector<FFT> ffts;
		ffts.emplace_back(config.fft.size);

		if(config.input.stereo){
			buffers.emplace_back(config.buf_size);
			ffts.emplace_back(config.fft.size);
		}

		Config_Monitor cm(config.get_file(), config_reload);
		// start input thread
		std::unique_ptr<Input_thread> inth = ::make_unique<Input_thread>(make_input(config.input), buffers);

		// attach SIGUSR1 signal handler
		std::signal(SIGUSR1, sighandler);

#ifndef WITH_TRANSPARENCY
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
		window = glfwCreateWindow(config.w_width, config.w_height, title.c_str(), NULL, NULL);
		if( window == NULL ){
			throw glfw_error("Failed to create GLFW Window!");
		}

		glfwMakeContextCurrent(window);
		glfwSwapInterval(-1);

		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetKeyCallback(window, key_callback);
#else
		GLXwindow window(config.w_width, config.w_height);

		{
			std::string title = generate_title(config);
			window.set_title(title);
		}

		// use adaptive vsync
		bool mesa_swap_control = GLXwindow::hasExt(window.glx_exts, "GLX_MESA_swap_control");
		if(mesa_swap_control){
			using glXSwapIntervalMESAProc = int (*)(int interval);
			glXSwapIntervalMESAProc glXSwapIntervalMESA = nullptr;
			glXSwapIntervalMESA = (glXSwapIntervalMESAProc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");
			glXSwapIntervalMESA(-1);
		}
#endif
		set_bg_color(config.bg_color);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		std::vector<Spectrum> spectra;
		std::vector<Oscilloscope> oscilloscopes;

		// create new renderers
		update_render_configs(spectra, config.spectra);
		update_render_configs(oscilloscopes, config.oscilloscopes);

		mainloop(config, window,
		[&]{
			// resize buffers and reconfigure renderer
			for(auto& buf : buffers){
				buf.resize(config.buf_size);
			}

			for(auto& fft : ffts){
				fft.resize(config.fft.size);
			}
			// check if input configuration has changed
			if(config.old_input != config.input){
				// stop input thread
				inth.reset(nullptr);
				
				// create new buffers/ffts
				if(config.input.stereo && buffers.size() < 2){
					buffers.emplace_back(config.buf_size);
					ffts.emplace_back(config.fft.size);
				}
				// destroy buffers/ffts
				if(!config.input.stereo && buffers.size() > 1){
					buffers.pop_back();
					ffts.pop_back();
				}
				
				// start new input thread
				inth.reset(new Input_thread(make_input(config.input), buffers));
			}

			update_render_configs(spectra, config.spectra);
			update_render_configs(oscilloscopes, config.oscilloscopes);

			set_bg_color(config.bg_color);
		},
		[&](const float dt){
			// update all locking renderer first
			for(unsigned i = 0; i < ffts.size(); i++){
				ffts[i].calculate(buffers[i]);
			}

			// test rms calculation
			//std::cout << "RMS: " << 20 * std::log10(normalize_rms(buffer.rms(), buffer.size, 1<<15)) << "dB" << std::endl;
			for(Oscilloscope& o : oscilloscopes){
				o.update_buffer(buffers);
			}
			// draw spectra and oscilloscopes
			for(Spectrum& s : spectra){
				s.update_fft(ffts);
				s.draw(dt);
			}
			for(Oscilloscope& o : oscilloscopes){
				o.draw();
			}
		});

	}catch(std::runtime_error& e){
		// print error message and terminate with error code 1
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}

inline float normalize_rms(float sum, float length, float amplitude){
	// rms normalization: divide sum by buffer length times 4^15(max amplitude)
	return std::sqrt(sum/(length*amplitude*amplitude));
}

Input::Ptr make_input(const Module_Config::Input& i){
// defines how many samples are read from the buffer during each loop iteration
// this number has to be even in stereo mode
const int SAMPLES = 220;

	// audio source configuration
	switch (i.source){
#ifdef WITH_PULSE
	case Module_Config::Source::PULSE:
		return ::make_unique<Pulse>(i.device, i.f_sample, SAMPLES, i.stereo);
#endif
	default:
		return ::make_unique<Fifo>(i.file, SAMPLES);
	}
}

std::string generate_title(const Config& config){
	std::stringstream title;
	title << "GLMViz:";
	if (config.spectra.size() > 0){
		title << " Spectrum (f_st=" << config.spec_default.data_offset * config.fft.d_freq << "Hz, \u0394f="
			<< config.spec_default.output_size * config.fft.d_freq << "Hz)";
	}
	if (config.oscilloscopes.size() > 0){
		title << " Oscilloscope (dur=" << config.duration << "ms)";
	}
	return title.str();
}

template <typename R_vector, typename C_vector>
void update_render_configs(R_vector& renderer, C_vector& configs){
	for(unsigned i = 0; i < configs.size(); i++){
		try{
			//reconfigure renderer
			renderer.at(i).configure(configs[i]);
		}catch(std::out_of_range& e){
			//make new renderer
			renderer.emplace_back(configs[i], i);
		}
	}
	//delete remaining renderers
	if(renderer.size() > configs.size()){
		renderer.erase(renderer.begin() + configs.size(), renderer.end());
	}
}
