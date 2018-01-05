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

#include <fstream>

#include "Input.hpp"
#include <thread>
#include <atomic>

#define DELAY_MIN 500
#define DELAY_MAX 5000

class Fifo : public Input{
public:
	explicit Fifo(Buffers::Ptr& buffers) : buffers(buffers){};

	~Fifo() override;

	void start_stream(const Module_Config::Input&) override;

	void stop_stream() override;
private:
	struct fifo_stream{
		std::atomic<bool> running;
		std::unique_ptr<int16_t[]> pre_buffer;
		int buffer_length;

		Buffers::Ptr& buffers;

		std::thread thread;
		std::ifstream file;

		int delay = 100;

		explicit fifo_stream(Buffers::Ptr&, const std::string&, const size_t);

		~fifo_stream();

		void read();
	};

	std::unique_ptr<fifo_stream> stream;
	Buffers::Ptr buffers;
};
