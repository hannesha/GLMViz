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