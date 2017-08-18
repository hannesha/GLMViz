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

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <thread>

#include <iostream>
#include <atomic>

// Include basic GL utility headers
#include "GL_utils.hpp"

#ifndef WITH_TRANSPARENCY
#include <GLFW/glfw3.h>
#else
#include "GLXwindow.hpp"
#endif

// Include helper files
#include "Program.hpp"
#include "FFT.hpp"
#include "Input.hpp"
#include "Fifo.hpp"
#include "Buffer.hpp"
#include "Config.hpp"
#include "Spectrum.hpp"
#include "Oscilloscope.hpp"

#ifdef WITH_PULSE
#include "Pulse.hpp"
#endif

// make_unique template for backwards compatibility
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// input thread wrapper
class Input_thread{
	public:
		// stereo constructor
		template <typename Buf> Input_thread(Input::Ptr&& i, std::vector<Buf>& buffers):
			running(true),
			input(std::move(i)),
			th_input([&]{
				while(running){
					input->read(buffers);
				};
			}){};

		~Input_thread(){ running = false; th_input.join(); };

	private:
		std::atomic<bool> running;
		Input::Ptr input;
		std::thread th_input;
};
