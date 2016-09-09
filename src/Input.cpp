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

#include "Input.hpp"

Input::Input(const char* file_name){
	handle = open(file_name, O_RDONLY | O_NONBLOCK);
}

Input::~Input(){
	close(handle);
}

bool Input::is_open() const {
	return handle >= 0;
}

void Input::read_fifo(std::vector<int16_t>& vbuf, FFT& fft){
	int16_t buf[vbuf.size()];
	int64_t data = read(handle, buf, sizeof(buf));
	if(data > 0){
		int64_t samples_read = data/sizeof(int16_t);
                vbuf.erase(vbuf.begin(), vbuf.begin() + samples_read);
                vbuf.insert(vbuf.end(), buf, buf + samples_read);

		fft.calculate(vbuf);
	}
}
