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
	input = static_cast<double *>(fftw_malloc(sizeof(double) * size));
	output = static_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * output_size));
	plan = fftw_plan_dft_r2c_1d(size, input, output, FFTW_ESTIMATE);
}

FFT::~FFT(){
	fftw_free(output);	
	fftw_free(input);
	fftw_destroy_plan(plan);
}

void FFT::calculate(const std::vector<int16_t>& data){
	// find smallest value for window function
	size_t window_size = size < data.size() ? size : data.size();
	
	if (window.size() != window_size){
		calculate_window(window_size);
	}
	
	unsigned int i;
	for(i = 0; i < window_size; i++){
		// apply hann window with corrected factors (a * 2)
		input[i] = (float) data[i] * window[i];
	}
	
	// pad remainig values
	for(; i < size; i++){
		input[i] = 0;
	}
	
	// execute fft
	fftw_execute(plan);
}

size_t FFT::get_size() const {
	return size;
}

void FFT::calculate_window(const size_t w_size){
	window.resize(w_size);
	double N_1 = 1.0 / (double)(w_size-1);
	
	for(unsigned int i = 0; i < w_size; i++){
		window[i] = 1.0 - cos(2.0 * M_PI * (double)i * N_1);
	}
}
