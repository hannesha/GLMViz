/*
 *	Copyright (C) 2016,2017  Hannes Haberl
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
#include <libconfig.h++>
#include "Module_Config.hpp"

class Config {
	public:
		Config(const std::string&);
		// disable copy construction
		Config(const Config&) = delete;
		void reload();

		int w_aa = 4;
		int w_height = 768;
		int w_width = 1024;

		Module_Config::Input input;
		Module_Config::Input old_input;

		int duration = 50;
		int fps = 60;

		bool show_fps = false;
		int show_fps_interval = 60;

		long long buf_size = input.f_sample * duration / 1000;

		Module_Config::FFT fft;

		Module_Config::Color bg_color = {0, 0, 0, 1};

		Module_Config::Oscilloscope osc_default;
		Module_Config::Spectrum spec_default;

		std::vector<Module_Config::Oscilloscope> oscilloscopes;
		std::vector<Module_Config::Spectrum> spectra;

		std::string get_file(){
			return file;
		}
	private:
		libconfig::Config cfg;
		std::string file;

		void parse_input(Module_Config::Input&, libconfig::Setting&);
		void parse_fft(Module_Config::FFT&, libconfig::Setting&);
		void parse_color(Module_Config::Color&, const std::string&, libconfig::Setting&);
		void parse_rainbow(Module_Config::Spectrum&, libconfig::Setting&);
		void parse_transformation(Module_Config::Transformation&, const std::string&, libconfig::Setting&);
		void parse_oscilloscope(Module_Config::Oscilloscope&, libconfig::Setting&);
		void parse_spectrum(Module_Config::Spectrum&, libconfig::Setting&, const Module_Config::FFT&);


		static const unsigned MAX_SPECTRA = 4;
		static const unsigned MAX_OSCILLOSCOPES = 4;
};
