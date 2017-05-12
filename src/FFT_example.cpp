/*
 *	Copyright (C) 2017  Hannes Haberl
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

#include <iostream>

#include "FFT.hpp"

int main(){

	// create buffer with 200 elements
	Buffer<int16_t> audio_buffer(200);

	// fill buffer with 100 1s
	audio_buffer.write(std::vector<int16_t>(100, 1));

	// create a fft with 1024 input elements (real data) and 513 complex elements on the output
	FFT fft(1024);

	// apply the fft on the audio buffer
	fft.calculate(audio_buffer);

	// calculate the magnitudes of the fft output
	// ! the magnitude for 16 bit data should be 32768 !
	auto mags = fft.magnitudes(1);

	for(auto m : mags){
		std::cout << m << "dB" << std::endl;
	}

	return 0;
}
