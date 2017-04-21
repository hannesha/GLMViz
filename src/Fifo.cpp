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

#include "Fifo.hpp"

#include <unistd.h>
#include <stdexcept>

Fifo::Fifo(const std::string& file_name, const size_t nsamples):buf(nsamples){
	samples = nsamples;
	file.open(file_name, std::ifstream::in | std::ifstream::binary);

	if(!file.is_open()) throw std::runtime_error("Unable to open FIFO file: " + file_name + " !");
}

bool Fifo::is_open() const {
	return file.is_open();
}

void Fifo::read(Buffer<int16_t>& buffer) const{
	file.read(reinterpret_cast<char *>(buf.data()), samples * sizeof(int16_t));

	buffer.write(buf);
}

void Fifo::read_stereo(Buffer<int16_t>& lbuffer, Buffer<int16_t>& rbuffer) const{
	file.read(reinterpret_cast<char *>(buf.data()), samples * sizeof(int16_t));

	lbuffer.write_offset(buf, 2, 0);
	rbuffer.write_offset(buf, 2, 1);
}
