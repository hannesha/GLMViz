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

#include "Fifo.hpp"

#include <stdexcept>
#include <chrono>

Fifo::~Fifo(){
	stop_stream();
};

void Fifo::stop_stream(){
	stream.reset(nullptr);
}

void Fifo::start_stream(const Module_Config::Input& input_config){
	stop_stream();

	stream.reset(new fifo_stream(buffers, input_config.file, input_config.latency));
}

Fifo::fifo_stream::fifo_stream(Buffers::Ptr& buffs, const std::string& filename, const size_t buff_len) :
		running(true),
		pre_buffer(new int16_t[buff_len]),
		buffer_length(buff_len),
		buffers(buffs){
	file.open(filename, std::ifstream::in | std::ifstream::binary);

	if(!file.is_open()) throw std::runtime_error("Unable to open FIFO file: " + filename + " !");

	thread = std::thread([&]{
		while (running){
			read();
		};
	});
}
Fifo::fifo_stream::~fifo_stream(){
	running = false;
	thread.join();
};

template<typename T>
inline T clamp(const T n, const T min, const T max){
	return std::max(min, std::min(n, max));
}

// internal read function
template<typename T>
static void i_read(std::vector<Buffer<T>>& buffers, T buf[], size_t size){
	size = size / 2;
	if(buffers.size() > 1){
		buffers[0].write_offset(buf, size, 2, 0);
		buffers[1].write_offset(buf, size, 2, 1);
	}else{
		buffers[0].write(buf, size);
	}
}

void Fifo::fifo_stream::read(){
	long s_read = file.readsome(reinterpret_cast<char*>(pre_buffer.get()), buffer_length * sizeof(int16_t));

	if(s_read > 0){
		std::lock_guard<std::mutex> lock(buffers->mut);
		i_read(buffers->bufs, pre_buffer.get(), s_read);
	}

	int diff = buffer_length - s_read;
	delay += diff / 4;
	delay = clamp(delay, DELAY_MIN, DELAY_MAX);
	std::this_thread::sleep_for(std::chrono::microseconds(delay));
}
