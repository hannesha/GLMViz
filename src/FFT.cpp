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

void FFT::calculate(Buffer &buffer){
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
			input[i] = (float) buffer.v_buffer[i] * window[i];
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

size_t FFT::get_size() const {
	return size;
}

void FFT::calculate_window(const size_t w_size){
	window.resize(w_size);
	float N_1 = 1.0 / (float)(w_size-1);
	
	for(unsigned int i = 0; i < w_size; i++){
		window[i] = 1.0 - cos(2.0 * M_PI * (float)i * N_1);
	}
}
