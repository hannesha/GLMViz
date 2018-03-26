/*
 *	Copyright (C) 2018 Hannes Haberl
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

#pragma once

namespace Util{
	/**
	 * Calculates the slope of a linear map function of the in/out parameters.
	 */
	template<typename T>
	inline T slope(const T in_min, const T in_max, const T out_min, const T out_max){
		return (out_max - out_min) / (in_max - in_min);
	}

	/**
	 * Calculates the offset of a linear map function of the in/out parameters.
	 */
	template<typename T>
	inline T offset(const T in_x, const T out_x, const T slope){
		return out_x - slope * in_x;
	}


	/**
	 * Calculates the buffer length for a given sample rate and duration.
	 */
	template<typename T, typename S>
	inline T buffer_size(const T sample_rate, const S duration){
		return static_cast<S>(sample_rate) * duration;
	}


	/**
	 * Calculates the normalization value of a fft with the given window_size (effective input data)
	 * and signal amplitude.
	 */
	template<typename T>
	inline T fft_scale(const T window_size, const T max_amplitude){
		return 1.0 / (window_size * max_amplitude);
	}

	/**
	 * Calculates the delta frequency of a fft.
	 */
	template<typename R, typename T>
	inline R fft_df(const T sample_rate, const T fft_size){
		return static_cast<R>(sample_rate) / fft_size;
	}
}
