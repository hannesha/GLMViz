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

#pragma once

#include <vector>
#include <stdint.h>
#include <mutex>

template<typename T>
class Buffer {
	public:
		Buffer(const size_t);

		std::vector<T> v_buffer;
		bool new_data;
		size_t size;

		std::unique_lock<std::mutex> lock();
		void write(T buf[], const size_t);
		void write(const std::vector<T>& buf);
		void write_offset(T buf[], const size_t, const size_t, const size_t);
		void write_offset(const std::vector<T>& buf, const size_t, const size_t);
		void resize(const size_t);
	private:
		std::mutex m;
};
