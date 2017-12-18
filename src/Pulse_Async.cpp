/*
 *	Copyright (C) 2017 Hannes Haberl
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

#include "Pulse_Async.hpp"

#include <stdexcept>
#include <iostream>


namespace PA{
	struct Lock{
		Lock(pa_threaded_mainloop* m) : mainloop(m){
			pa_threaded_mainloop_lock(mainloop);
		}

		~Lock(){
			if(mainloop)
				pa_threaded_mainloop_unlock(mainloop);
		}

		void unlock(){
			if(mainloop){
				pa_threaded_mainloop_unlock(mainloop);
				mainloop = nullptr;
			}
		}

	private:
		pa_threaded_mainloop* mainloop;
	};
}

Pulse_Async::Pulse_Async(Buffers::Ptr& p_buffers) : stream(nullptr){
	// make threaded mainloop
	pa_threaded_mainloop* mainloop = pa_threaded_mainloop_new();
	if(!mainloop){

		throw std::runtime_error("Can't create Pulseaudio mainloop!");
	}


	PA::Lock lock(mainloop);
	pa_context* context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "GLMViz (async)");
	if(!context){
		lock.unlock();
		pa_threaded_mainloop_free(mainloop);
		throw std::runtime_error("Can't create Pulseaudio context!");
	}

	userdata = std::unique_ptr<usr_data>(new usr_data);
	userdata->device = "";
	userdata->p_buffers = p_buffers;
	userdata->mainloop = mainloop;
	userdata->context = context;

	pa_context_set_state_callback(context, state_cb, userdata.get());
	if(pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0 || pa_threaded_mainloop_start(mainloop) < 0){
		pa_context_disconnect(context);
		pa_context_unref(context);
		lock.unlock();
		pa_threaded_mainloop_free(mainloop);
		throw std::runtime_error("Can't connect Pulseaudio context!");
	}

	pa_threaded_mainloop_wait(mainloop);
	if(pa_context_get_state(context) != PA_CONTEXT_READY){
		pa_threaded_mainloop_stop(mainloop);
		pa_context_disconnect(context);
		pa_context_unref(context);
		lock.unlock();
		pa_threaded_mainloop_free(mainloop);
	}

	// get server info
	pa_operation* operation;
	operation = pa_context_get_server_info(context, Pulse_Async::info_cb, userdata.get());
	while (pa_operation_get_state(operation) != PA_OPERATION_DONE){
		pa_threaded_mainloop_wait(mainloop);
	}
	pa_operation_unref(operation);
}

Pulse_Async::~Pulse_Async(){
	stop_stream();
	pa_threaded_mainloop_stop(userdata->mainloop);
	pa_context_disconnect(userdata->context);
	pa_context_unref(userdata->context);

	pa_threaded_mainloop_free(userdata->mainloop);
}

void Pulse_Async::state_cb(pa_context* context, void* userdata){
	struct Pulse_Async::usr_data* data = reinterpret_cast<struct Pulse_Async::usr_data*>(userdata);
	switch (pa_context_get_state(context)){
		case PA_CONTEXT_READY:
		case PA_CONTEXT_TERMINATED:
		case PA_CONTEXT_FAILED:
			// signal mainloop
			pa_threaded_mainloop_signal(data->mainloop, 0);
			break;

		default:
			break;
	}
}

void Pulse_Async::info_cb(pa_context* context, const pa_server_info* info, void* userdata){
	struct Pulse_Async::usr_data* data = reinterpret_cast<struct Pulse_Async::usr_data*>(userdata);
	//get default sink name
	data->device = info->default_sink_name;
	//append .monitor sufix
	data->device.append(".monitor");

	pa_threaded_mainloop_signal(data->mainloop, 0);
}

void Pulse_Async::stop_stream(){
	if(stream){
		PA::Lock lock(userdata->mainloop);

		pa_stream_disconnect(stream);
		pa_stream_unref(stream);
		stream = nullptr;

		//std::cout << "Stopping stream!" << std::endl;
	}
}

void Pulse_Async::start_stream(const Module_Config::Input &config){
	PA::Lock lock(userdata->mainloop);

	pa_sample_spec sample_spec = {
			format : PA_SAMPLE_S16LE,
			rate :  (uint32_t) config.f_sample,
			channels : (uint8_t) (config.stereo ? 2 : 1)
	};


	stream = pa_stream_new(userdata->context, "GLMViz input", &sample_spec, nullptr);
	if(!stream){
		throw std::runtime_error("Can't create Audio stream!");
	}


	pa_stream_set_state_callback(stream, stream_state_cb, userdata.get());
	pa_stream_set_read_callback(stream, stream_read_cb, userdata.get());

	std::string dev;
	if(config.device == ""){
		dev = userdata->device;
	}else{
		dev = config.device;
	}

	//TODO: add latency parameter
	pa_buffer_attr buffer_attr = {
			(uint32_t) -1,
			(uint32_t) -1,
			(uint32_t) -1,
			(uint32_t) -1,
			(uint32_t) 1100,
	};


	if(pa_stream_connect_record(stream, dev.c_str(), &buffer_attr, PA_STREAM_ADJUST_LATENCY) < 0){
		throw std::runtime_error("Can't connect Audio stream!");
	}

	pa_threaded_mainloop_wait(userdata->mainloop);
	if(pa_stream_get_state(stream) != PA_STREAM_READY){
		throw std::runtime_error("Can't connect Audio stream!");
	}

	//std::cout << "starting stream with: " << (int)sample_spec.channels << " channels" << std::endl;
}

void Pulse_Async::stream_state_cb(pa_stream* stream, void* userdata){
	struct Pulse_Async::usr_data* data = reinterpret_cast<struct Pulse_Async::usr_data*>(userdata);
	switch (pa_stream_get_state(stream)){
		case PA_STREAM_READY:
		case PA_STREAM_TERMINATED:
		case PA_STREAM_FAILED:
			// signal mainloop
			pa_threaded_mainloop_signal(data->mainloop, 0);
			break;

		default:
			break;
	}
}

// internal read function
template<typename T>
static void i_read(std::vector<Buffer<T>> &buffers, T buf[], size_t size){
	size = size / 2;
	if(buffers.size() > 1){
		buffers[0].write_offset(buf, size, 2, 0);
		buffers[1].write_offset(buf, size, 2, 1);
	}else{
		buffers[0].write(buf, size);
	}
}

void Pulse_Async::stream_read_cb(pa_stream* stream, size_t len, void* userdata){
	struct Pulse_Async::usr_data* data = reinterpret_cast<struct Pulse_Async::usr_data*>(userdata);

	while (pa_stream_readable_size(stream)){
		// read stream buffer
		int16_t* buf;
		if(pa_stream_peek(stream, (const void**) &buf, &len) < 0){
			return;
		}

		if(buf){
			// lock buffer vector, read stream buffer into audio buffers
			std::lock_guard<std::mutex> lock(data->p_buffers->mut);
			i_read(data->p_buffers->bufs, buf, len);
		}

		// drop stream buffer
		pa_stream_drop(stream);
	}
}