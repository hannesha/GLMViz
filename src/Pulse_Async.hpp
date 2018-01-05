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

#pragma once

#include "Input.hpp"
#include <pulse/pulseaudio.h>

#ifdef WITH_PULSE
#pragma message("Pulse support enabled")
#endif

class Pulse_Async : public Input{
public:
	explicit Pulse_Async(Buffers::Ptr&);

	~Pulse_Async() override;

	void start_stream(const Module_Config::Input&) override;

	void stop_stream() override;

private:
	static void state_cb(pa_context*, void*);

	static void info_cb(pa_context*, const pa_server_info*, void*);

	static void stream_state_cb(pa_stream*, void*);

	static void stream_read_cb(pa_stream*, size_t, void*);

	std::string device;
	pa_threaded_mainloop* mainloop;
	pa_context* context;
	Buffers::Ptr p_buffers;
	pa_stream* stream;
};
