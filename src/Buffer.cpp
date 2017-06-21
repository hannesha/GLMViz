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

#include "Buffer.hpp"
#include <type_traits>
#include <algorithm>

template<typename T>
Buffer<T>::Buffer(const size_t size){
	static_assert(std::is_arithmetic<T>::value, "Buffer<T> only supports arithmetic types!");

	v_buffer.resize(size);
	this->size = size;
	new_data = true;
}

template<typename T>
std::unique_lock<std::mutex> Buffer<T>::lock(){
	return std::unique_lock<std::mutex>(m);
}

template<typename T>
void Buffer<T>::write(T buf[], const size_t n){
	auto lock = this->lock();
	new_data = true;

	// limit data to write
	size_t length = std::min(n, size);
	v_buffer.erase(v_buffer.begin(), v_buffer.begin() + length);
	v_buffer.insert(v_buffer.end(), buf, buf + length);
}

template<typename T>
void Buffer<T>::write(const std::vector<T>& buf){
	auto lock = this->lock();
	new_data = true;

	// limit data to write
	size_t length = std::min(buf.size(), size);
	v_buffer.erase(v_buffer.begin(), v_buffer.begin() + length);
	v_buffer.insert(v_buffer.end(), buf.begin(), buf.begin() + length);
}

template<typename T>
void Buffer<T>::write_offset(T buf[], const size_t n, const size_t gap, const size_t offset){
	auto lock = this->lock();
	new_data = true;

	// limit data to write
	size_t length = std::min(n, size);
	size_t written = 0;
	for(size_t i = offset; i<length; i += gap){
		v_buffer.push_back(buf[i]);
		written++;
	}
	v_buffer.erase(v_buffer.begin(), v_buffer.begin() + written);
}

template<typename T>
void Buffer<T>::write_offset(const std::vector<T>& buf, const size_t gap, const size_t offset){
	auto lock = this->lock();
	new_data = true;

	// limit data to write
	size_t length = std::min(buf.size(), size);
	size_t written = 0;
	for(size_t i = offset; i<length; i += gap){
		v_buffer.push_back(buf[i]);
		written++;
	}
	v_buffer.erase(v_buffer.begin(), v_buffer.begin() + written);
}

template<typename T>
void Buffer<T>::resize(const size_t n){
	auto lock = this->lock();
	if(size != n){
		size = n;
		v_buffer.resize(n);
		new_data = true;
	}
}

// calculate the rms value of all the audio data in the buffer
template <typename T>
float Buffer<T>::rms(){
	auto lock = this->lock();
	float rms = 0;
	unsigned i = 0;

	constexpr unsigned N = 4;
	// temp sum vector
	alignas(16) std::array<float,N> vrms;
	for(unsigned k=0; k<N; k++) vrms[k] = 0;

	for(; i < size/N; i++){
		// load 4 integers and convert them to float
		alignas(16) std::array<float, N> vdata;
		for(unsigned k=0; k<N; k++) vdata[k] = (float)v_buffer[i*N + k];

		#pragma omp simd
		for(unsigned k=0; k<N; k++) vrms[k] += vdata[k] * vdata[k];
	}
	// calculate sum
	for(unsigned k=0; k<N; k++) rms += vrms[k];

	// calculate remaining values
	i *= N;
	for(; i < size; i++){
		 rms += v_buffer[i] * v_buffer[i];
	}
	return rms;
}

template class Buffer<int16_t>;
