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

#include "FFT.hpp"

FFT::FFT(const size_t fft_size){
	size = fft_size;
	size_t output_size = size/2+1;
	input = reinterpret_cast<float*>(fftwf_malloc(sizeof(float) * size));
	output = reinterpret_cast<fftwf_complex*>(fftwf_malloc(sizeof(fftwf_complex) * output_size));
	plan = fftwf_plan_dft_r2c_1d(size, input, output, FFTW_ESTIMATE);
}

FFT::~FFT(){
	fftwf_free(output);
	fftwf_free(input);
	fftwf_destroy_plan(plan);
}

void FFT::resize(const size_t nsize){
	if(size != nsize){
		size = nsize;
		// destroy old plan and free memory
		fftwf_destroy_plan(plan);
		fftwf_free(input);
		fftwf_free(output);

		// create new plan and allocate memory
		input = reinterpret_cast<float*>(fftwf_malloc(sizeof(float) * size));
		size_t output_size = size/2+1;
		output = reinterpret_cast<fftwf_complex*>(fftwf_malloc(sizeof(fftwf_complex) * output_size));
		plan = fftwf_plan_dft_r2c_1d(size, input, output, FFTW_ESTIMATE);
	}
}

template<typename T>
void FFT::calculate(Buffer<T>& buffer){
	// find smallest value for window function
	size_t window_size = std::min(size, buffer.size);

	if (window.size() != window_size){
		calculate_window(window_size);
	}

	auto lock = buffer.lock();
	if(buffer.new_data){
		buffer.new_data = false;

		unsigned int i;
		for(i = 0; i < window_size; i++){
			// apply hann window with corrected factors (a * 2)
			input[i] = static_cast<float>(buffer.v_buffer[i]) * window[i];
		}

		lock.unlock();

		// pad remainig values
		for(; i < size; i++){
			input[i] = 0;
		}

		// execute fft
		fftwf_execute(plan);
	}
}

// return the index of the bin with the highest magnitude
size_t FFT::max_bin(const size_t start, const size_t stop){
	size_t startl = std::min(size, start);
	size_t stopl = std::min(size, stop);
	if(startl > stopl) return stopl;

	size_t ret = startl;
	float max = 0;
	for(size_t i = startl; i < stopl; i++){
		float mag = std::hypot(output[i][0], output[i][1]);
		if(mag > max){
			max = mag;
			ret = i;
		}
	}

	return ret;
}

// calculate the magnitude(in dB) of the fft output
// max_amplitude specified the maximum value of the fft input (32768 for a 16 bit audio signal)
std::vector<float> FFT::magnitudes(const float max_amplitude){
	std::vector<float> mag(size/2 +1);

	float scale = 1./ ((float)(window.size()/2 +1) * max_amplitude);

	for(unsigned i = 0; i < size/2+1; i++){
		mag[i] = 20. * std::log10(std::hypot(output[i][0], output[i][1]) * scale);
	}

	return mag;
}

void FFT::calculate_window(const size_t w_size){
	window.resize(w_size);
	float N_1 = 1.0 / (float)(w_size-1);

	// compensated Blackman window constants
	const float a1 = 4620.0 / 3969.0;
	const float a2 = 715.0 / 3969.0;

	for(unsigned int i = 0; i < w_size; i++){
		/* Hann window */
		//window[i] = 1.0 - cos(2.0 * M_PI * (float)i * N_1);

		/* exact Blackman window */
		window[i] = 1.0 - a1 * cos(2*M_PI * i * N_1) + a2 * cos(4*M_PI * i * N_1);
	}
}

template void FFT::calculate(Buffer<int16_t>&);
