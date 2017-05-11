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

#include "Pulse.hpp"

//#include <pulse/error.h>
#include <stdexcept>
#include <sstream>

#define FRAG_SIZE nsamples*10

Pulse::Pulse(const std::string& device, const size_t FS, const size_t nsamples, const bool stereo):buf(new int16_t[nsamples]){
	samples = nsamples;
	unsigned char ch = stereo ? 2 : 1;

	pa_sample_spec sample_spec = {
		format : PA_SAMPLE_S16LE,
		rate :  (uint32_t)FS,
		channels : ch
	};

	pa_buffer_attr buffer_attr = {
				(uint32_t)-1,	//maxlength
				(uint32_t)-1,
				(uint32_t)-1,
				(uint32_t)-1,
				static_cast<uint32_t>(FRAG_SIZE)	//fragsize
        };

	std::string default_device = device;
	if(default_device == ""){
		default_device = get_default_sink();
	}

	int error;
	stream = pa_simple_new(nullptr, "GLMViz", PA_STREAM_RECORD, default_device.c_str(), "GLMViz monitor", &sample_spec, nullptr, &buffer_attr, &error);

	if(stream == nullptr){
		std::stringstream msg;
		msg << "PulseAudio initialization failed with error: " << pa_strerror(error) << " !";
		throw std::runtime_error(msg.str());
	}
}

Pulse::~Pulse(){
	pa_simple_free(stream);
}

bool Pulse::is_open() const{
	return stream != nullptr;
}

void Pulse::read(Buffer<int16_t>& buffer) const{
	pa_simple_read(stream, buf.get(), samples * sizeof(int16_t), NULL);
	buffer.write(buf.get(), samples);
}

void Pulse::read_stereo(Buffer<int16_t>& lbuffer, Buffer<int16_t>& rbuffer) const{
	pa_simple_read(stream, buf.get(), samples * sizeof(int16_t), NULL);
	lbuffer.write_offset(buf.get(), samples, 2, 0);
	rbuffer.write_offset(buf.get(), samples, 2, 1);
}

std::string Pulse::get_default_sink(){
	std::string device;
	pa_mainloop_api *mainloop_api;
	pa_context *context;
	pa_mainloop *mainloop;

	// Create a mainloop API and connection to the default server
	mainloop = pa_mainloop_new();

	mainloop_api = pa_mainloop_get_api(mainloop);
	context = pa_context_new(mainloop_api, "Viz device list");


	pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);

	// init userdata struct
	struct Pulse::usr_data data {&device, mainloop};

	// Set callback and hand through the device string and mainloop
	pa_context_set_state_callback(context, Pulse::state_cb, &data);

	int err;
	//starting a mainloop to get default sink
	if (pa_mainloop_run(mainloop, &err) < 0){
		std::stringstream msg;
		msg << "Could not open pulseaudio mainloop to find default device name: " << err << "\n";
		throw std::runtime_error(msg.str());
	}

	pa_context_disconnect(context);
	pa_context_unref(context);
	pa_mainloop_free(mainloop);
	return device;
}

void Pulse::state_cb(pa_context* context, void* userdata){
	//make sure loop is ready
	switch (pa_context_get_state(context)){
		case PA_CONTEXT_READY:
		// read server info
		pa_operation_unref(pa_context_get_server_info(
			context, Pulse::info_cb, userdata));
		default:
		break;
	}
}

void Pulse::info_cb(pa_context* context, const pa_server_info* info, void* userdata){
	// recast userdata struct
	struct Pulse::usr_data *data = reinterpret_cast<struct Pulse::usr_data*>(userdata);
	//get default sink name
	*(data->device) = std::string(info->default_sink_name);

	//append .monitor sufix
	data->device->append(".monitor");

	//quit mainloop
	pa_mainloop_quit(data->mainloop, 0);
}
