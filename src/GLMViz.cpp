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
#include <atomic>

// make_unique template for backwards compatibility
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

//typedef std::unique_ptr<Spectrum> Spec_ptr;
//typedef std::unique_ptr<Oscilloscope> Osc_ptr;

// input thread wrapper
class Input_thread{
	public:
		// mono constructor
		template <typename Buf> Input_thread(Input& i, Buf& buffer):
			running(true),
			th_input([&]{
				while(running){
					i.read(buffer);
				};
			}){};

		// stereo constructor
		template <typename Buf> Input_thread(Input& i, std::vector<Buf>& buffers):
			running(true),
			th_input([&]{
				while(running){
					i.read(buffers);
				};
			}){};

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
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
//	if(key == GLFW_KEY_R && action == GLFW_PRESS){
//		config_reload = true;
//	}
//}

// handle window resizing
//oid framebuffer_size_callback(GLFWwindow* window, int width, int height){
//	glViewport(0, 0, width, height);
//}

// set glClear color
void set_bg_color(const Config::Color& color){
	glClearColor(color.rgba[0], color.rgba[1], color.rgba[2], color.rgba[3]);
}

std::string generate_title(Config& config){
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

// create or delete renderers to match the corresponding configs
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

bool closing = false;
// mainloop template
template <typename Fupdate, typename Fdraw>
void mainloop(Config& config, GLXwindow& window, Fupdate f_update, Fdraw f_draw){
	Atom wm_delete_window = XInternAtom(window.display, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(window.display, window.win, &wm_delete_window, 1);

	int width = config.w_width;
	int height = config.w_height;
	// create multisample framebuffer
	GL::Multisampler msaa(config.w_aa, width, height);
	glEnable(GL_MULTISAMPLE);

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

		std::chrono::time_point<std::chrono::steady_clock> t_fps = std::chrono::steady_clock::now() + std::chrono::microseconds(1000000 / config.fps -100);

		msaa.bind();
		glClear(GL_COLOR_BUFFER_BIT);

		// draw
		f_draw();

		// Swap buffers
		msaa.blit(width, height);
		window.swapBuffers();
		//glfwPollEvents();

		// wait for fps timer
		std::this_thread::sleep_until(t_fps);
	}
}

// defines how many samples are read from the buffer during each loop iteration
// this number has to be even in stereo mode
#define SAMPLES 220

int main(int argc, char *argv[]){
	try{
		// construct config file path from first argument
		std::string config_file = "";
		if(argc > 1){
			config_file = argv[1];
		}
		// read config
		Config config(config_file);

		Input::Ptr input;

		// audio source configuration
		switch (config.input.source){
#ifdef WITH_PULSE
		case Source::PULSE:
			input = ::make_unique<Pulse>(config.input.device, config.input.f_sample, SAMPLES, config.input.stereo);
			break;
#endif
		default:
			input = ::make_unique<Fifo>(config.input.file, SAMPLES);
		}

		// attach SIGUSR1 signal handler
		std::signal(SIGUSR1, sighandler);

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

		set_bg_color(config.bg_color);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		std::vector<Spectrum> spectra;
		std::vector<Oscilloscope> oscilloscopes;

		// create new renderers
		update_render_configs(spectra, config.spectra);
		update_render_configs(oscilloscopes, config.oscilloscopes);

		if(!config.input.stereo){
			// create audio buffer
			Buffer<int16_t> buffer(config.buf_size);

			// start input thread
			Input_thread inth(*input, buffer);

			// create fft
			FFT fft(config.fft.size);

			// mono mainloop
			mainloop(config, window,
			[&]{
				// resize buffers and reconfigure renderer
				buffer.resize(config.buf_size);

				// resize fft
				fft.resize(config.fft.size);

				update_render_configs(spectra, config.spectra);
				update_render_configs(oscilloscopes, config.oscilloscopes);

				set_bg_color(config.bg_color);
			},
			[&]{
				// update all locking renderer first
				fft.calculate(buffer);
				for(Oscilloscope& o : oscilloscopes){
					o.update_buffer(buffer);
				}
				// draw spectra and oscilloscopes
				for(Spectrum& s : spectra){
					s.update_fft(fft);
					s.draw();
				}
				for(Oscilloscope& o : oscilloscopes){
					o.draw();
				}
			});
		}else{
			// create left and right audio buffer
			std::vector<Buffer<int16_t>> buffers;

			//Buffer<int16_t> lbuffer(config.buf_size);
			//Buffer<int16_t> rbuffer(config.buf_size);
			buffers.emplace_back(config.buf_size);
			buffers.emplace_back(config.buf_size);


			// start stereo input thread
			Input_thread inth(*input, buffers);

			// create FFT vector
			std::vector<FFT> ffts;
			ffts.emplace_back(config.fft.size);
			ffts.emplace_back(config.fft.size);

			// stereo mainloop
			mainloop(config, window,
			[&]{
				// resize buffers
				for(auto& buf : buffers){
					buf.resize(config.buf_size);
				}

				for(auto& fft : ffts){
					fft.resize(config.fft.size);
				}

				// update spectrum/oscilloscope renderer
				update_render_configs(spectra, config.spectra);
				update_render_configs(oscilloscopes, config.oscilloscopes);

				set_bg_color(config.bg_color);
			},
			[&]{
				ffts[0].calculate(buffers[0]);
				ffts[1].calculate(buffers[1]);
				for(Oscilloscope& o : oscilloscopes){
					o.update_buffer(buffers);
				}

				for(Spectrum& s : spectra){
					s.update_fft(ffts);
					s.draw();
				}

				for(Oscilloscope& o : oscilloscopes){
					o.draw();
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
