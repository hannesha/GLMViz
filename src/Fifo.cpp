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
#include <thread>
#include <chrono>

Fifo::Fifo(const std::string& file_name, const size_t nsamples):buf(new int16_t[nsamples]){
	samples = nsamples;
	file.open(file_name, std::ifstream::in | std::ifstream::binary);

	if(!file.is_open()) throw std::runtime_error("Unable to open FIFO file: " + file_name + " !");
}

bool Fifo::is_open() const {
	return file.is_open();
}

template<typename T>
inline T clamp(const T n, const T min, const T max){
	return std::max(min, std::min(n, max));
}

void Fifo::read(Buffer<int16_t>& buffer) const{
	size_t s_read = file.readsome(reinterpret_cast<char *>(buf.get()), samples * sizeof(int16_t));

	buffer.write(buf.get(), s_read/2);
	int diff = samples - s_read;
	delay += diff/4;
	delay = clamp(delay, DELAY_MIN, DELAY_MAX);
	std::this_thread::sleep_for(std::chrono::microseconds(delay));
}

void Fifo::read_stereo(Buffer<int16_t>& lbuffer, Buffer<int16_t>& rbuffer) const{
	size_t s_read = file.readsome(reinterpret_cast<char *>(buf.get()), samples * sizeof(int16_t));

	lbuffer.write_offset(buf.get(), s_read/2, 2, 0);
	rbuffer.write_offset(buf.get(), s_read/2, 2, 1);

	int diff = samples - s_read;
	delay += diff/8;
	delay = clamp(delay, DELAY_MIN, DELAY_MAX*2);
	std::this_thread::sleep_for(std::chrono::microseconds(delay));
}
