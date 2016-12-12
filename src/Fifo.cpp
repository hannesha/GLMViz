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

#include "Fifo.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

Fifo::Fifo(const std::string& file_name, const size_t nsamples){
	samples = nsamples;
	handle = open(file_name.c_str(), O_RDONLY);

	if(handle < 0) throw std::runtime_error("Unable to open FIFO file: " + file_name + " !");
}

Fifo::~Fifo(){
	close(handle);
}

bool Fifo::is_open() const {
	return handle >= 0;
}

void Fifo::read(Buffer<int16_t>& buffer) const{
	int16_t buf[samples];
	::read(handle, buf, sizeof(buf));

	buffer.write(buf, samples);
}

void Fifo::read_stereo(Buffer<int16_t>& lbuffer, Buffer<int16_t>& rbuffer) const{
	int16_t buf[samples];
	::read(handle, buf, sizeof(buf));

	lbuffer.write_offset(buf, samples, 2, 0);
	rbuffer.write_offset(buf, samples, 2, 1);
}
