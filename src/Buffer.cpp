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

#include "Buffer.hpp"

Buffer::Buffer(size_t size){
	v_buffer.resize(size);
	this->size = size;
	new_data = true;
}

std::unique_lock<std::mutex> Buffer::lock(){
	return std::unique_lock<std::mutex>(m);
}

void Buffer::write(int16_t buf[], size_t n){
	auto lock = this->lock();
	new_data = true;
	
	v_buffer.erase(v_buffer.begin(), v_buffer.begin() + n);
	v_buffer.insert(v_buffer.end(), buf, buf + n);
}
