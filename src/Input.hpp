#pragma once

#include <vector>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "FFT.hpp"

class Input {
	public:
		Input(const char*);
		~Input();
		bool is_open() const;
		void read_fifo(std::vector<int16_t>&, FFT&);
	private:
		int handle;
};
