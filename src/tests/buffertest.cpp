/*
 *	Copyright (C) 2017 Hannes Haberl
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
#include <numeric>
#include <algorithm>
#include <stdexcept>

#include "Buffer.hpp"

template<typename T>
inline void dump(Buffer<T>& buffer){
	auto lock = buffer.lock();
	for(auto& i : buffer.v_buffer){
		std::cout << i << std::endl;
	}
}

template<typename T>
inline void dump(const std::vector<T>& buffer){
	for(auto& i : buffer){
		std::cout << i << std::endl;
	}
}

template<typename T>
inline bool neq(const Buffer<T>& buf, std::vector<T>& vec){
	return !std::equal(buf.v_buffer.begin(), buf.v_buffer.end(), vec.begin());
}

const size_t len = 10;
int main(){
	try{
		Buffer<int16_t> buf(len);
		std::cout << "Basic write" << std::endl;
		{
			std::vector<int16_t> nums(len);
			std::iota(nums.begin(), nums.end(), 0);
			buf.write(nums);
			if(neq(buf, nums)) throw std::runtime_error("Basic write");
		}
		
		std::cout << "Append" << std::endl;
		buf.write({10,11,12});
		{
			std::vector<int16_t> result(len);
			std::iota(result.begin(), result.end(), 3);
			if(neq(buf, result)) throw std::runtime_error("Append");
		}
		
		std::cout << "Interleaved append" << std::endl;
		buf.write_offset({13,14,15,16,17}, 2, 0);
		{
			std::vector<int16_t> result = {6,7,8,9,10,11,12,13,15,17};
			if(neq(buf, result)) throw std::runtime_error("Interleaved append");
		}

		std::cout << "Interleaved append with offset" << std::endl;
		buf.write_offset({13,14,15,16,17}, 2, 1);
		{
			std::vector<int16_t> result = {8,9,10,11,12,13,15,17,14,16};
			if(neq(buf, result)) throw std::runtime_error("Interleaved append with offset");
		}

		std::cout << "Resize buffer" << std::endl;
		const int nsize = 5;
		buf.resize(nsize);
		if(buf.size != nsize || buf.v_buffer.size() != nsize) throw std::runtime_error("Resize buffer");
		
		std::cout << "Basic write pointer" << std::endl;
		{
			int16_t pdata[] = {1, 2, 3, 4, 5};
			buf.write(pdata, 5);
			std::vector<int16_t> result = {1,2,3,4,5};
			if(neq(buf, result)) throw std::runtime_error("Basic write pointer");
		}

		std::cout << "Append pointer" << std::endl;
		{
			int16_t pdata[] = {6, 7};
			buf.write(pdata, 2);
			std::vector<int16_t> result = {3,4,5,6,7};
			if(neq(buf, result)) throw std::runtime_error("Append pointer");
		}

		std::cout << "Interleaved append pointer" << std::endl;
		{
			int16_t pdata[] = {8, 9, 10, 11, 12};
			buf.write_offset(pdata, 5, 2, 0);
			std::vector<int16_t> result = {6,7,8,10,12};
			if(neq(buf, result)) throw std::runtime_error("Interleaved append pointer");
		}

		std::cout << "Interleaved append with offset pointer" << std::endl;
		{
			int16_t pdata[] = {8, 9, 10, 11, 12};
			buf.write_offset(pdata, 5, 2, 1);
			std::vector<int16_t> result = {8,10,12,9,11};
			if(neq(buf, result)) throw std::runtime_error("Interleaved append with offset pointer");
		}

	}
	catch(std::runtime_error& e){
		std::cerr << e.what() << " Failed!" << std::endl;
		return 1;
	}
	return 0;
}
