#pragma once

// Include FFTW3
#include <fftw3.h>

#include <vector>
#include <math.h>
#include <stdint.h>

class FFT {
	public:
		FFT(const size_t);
		~FFT();

		void calculate(const std::vector<int16_t>&);
		size_t get_size() const;
		fftw_complex* output;
	private:
		double* input;
		fftw_plan plan;
		size_t size;
		
		void calculate_window(const size_t);
		std::vector<double> window;
};
