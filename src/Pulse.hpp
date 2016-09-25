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

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include <string>

#include "Buffer.hpp"
#include "Input.hpp"

#ifdef WITH_PULSE
#pragma message("Pulse support enabled")
#endif

class Pulse : public Input{
	public:
		Pulse(const std::string&, const size_t);
		~Pulse();
		
		bool is_open() const;
		void read(Buffer&) const;

		static std::string get_default_sink();
		struct usr_data{
			std::string* device;
			pa_mainloop* mainloop;
		};
	private:
		size_t samples;

		static void info_cb(pa_context*, const pa_server_info*, void*);
		static void state_cb(pa_context* , void*);

		pa_simple *stream;
};
