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

#include "Config.hpp"

#include "xdg.hpp"
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cmath>

Config::Config(const std::string& config_file){
	if(config_file != ""){
		if(xdg::verify_path(config_file)){
			file = config_file;
		}else{
			file = xdg::find_config("/GLMViz/" + config_file);
			if (file == ""){
				std::cerr << "The specified config file doesn't exist, falling back to default config!" << std::endl;
			}
		}
	}

	if(file == "") file = xdg::find_config("/GLMViz/config");

	if(file == "") file = "/etc/GLMViz/config";

	reload();
	old_input = input;
}

void Config::reload(){
	try{
		cfg.readFile(file.c_str());

		cfg.lookupValue("Window.AA", w_aa);
		cfg.lookupValue("Window.height", w_height);
		cfg.lookupValue("Window.width", w_width);

		old_input = input;
		try{
			parse_input(input, cfg.lookup("Input"));
		}catch(const libconfig::SettingNotFoundException& e){}

		cfg.lookupValue("duration", duration);
		cfg.lookupValue("fps", fps);
		cfg.lookupValue("fft_size", fft.size);

		cfg.lookupValue("show_fps", show_fps);
		cfg.lookupValue("show_fps_interval", show_fps_interval);

		buf_size = input.f_sample * duration / 1000;
		fft.output_size = fft.size/2+1;
		fft.d_freq = (float) input.f_sample / (float) fft.size;

		parse_color(bg_color, "bg_color", cfg.getRoot());

		// normalization value for the fft output
		// calculate effective fft input data size
		float isize = std::min(buf_size, fft.size)/2+1;
		fft.scale = 1.0f/(isize*32768.0f);

		try{
			parse_oscilloscope(osc_default, cfg.lookup("Osc"));
		}
		catch(const libconfig::SettingNotFoundException& e){}

		for(unsigned i = 0; i < MAX_OSCILLOSCOPES; i++){
			std::string path = "Osc" + std::to_string(i+1);
			try{
				// init new Oscilloscope with default parameters
				Module_Config::Oscilloscope tmp = osc_default;
				// parse Oscilloscope and add it to the list
				parse_oscilloscope(tmp, cfg.lookup(path));

				try{
					// reuse old values if possible
					oscilloscopes.at(i) = tmp;
				}
				catch(std::out_of_range& e){
					oscilloscopes.push_back(tmp);
				}
			}
			catch(const libconfig::SettingNotFoundException& e){
				// delete remaining oscilloscopes
				if(oscilloscopes.size() > i){
					oscilloscopes.erase(oscilloscopes.begin() + i, oscilloscopes.end());
				}
				break;
			}
		}

		try{
			parse_spectrum(spec_default, cfg.lookup("Spectrum"), fft);
		}
		catch(const libconfig::SettingNotFoundException& e){}

		for(unsigned i = 0; i < MAX_SPECTRA; i++){
			std::string path = "Spectrum" + std::to_string(i+1);
			try{
				// init new Spectrum with default parameters
				Module_Config::Spectrum tmp = spec_default;
				// parse Spectrum and add it to the list
				parse_spectrum(tmp, cfg.lookup(path), fft);

				// reuse old values if possible
				try{
					spectra.at(i) = tmp;
				}
				catch(std::out_of_range& e){
					spectra.push_back(tmp);
				}
			}
			catch(const libconfig::SettingNotFoundException& e){
				// delete remaining spectra
				if(spectra.size() > i){
					spectra.erase(spectra.begin() + i, spectra.end());
				}
				break;
			}
		}

		//std::cout << oscilloscopes.size() << std::endl;
		//std::cout << spectra.size() << std::endl;

	}catch(const libconfig::FileIOException &fioex){
		std::cerr << "I/O error while reading file." << std::endl;

		std::cout << "Using default settings." << std::endl;
	}catch(const libconfig::ParseException &pex){
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
		<< " - " << pex.getError() << std::endl;

		std::cout << "Using default settings." << std::endl;
	}

	// render default spectrum when using a blank/faulty config
	if(spectra.size() == 0 && oscilloscopes.size() == 0){
		spectra.push_back(spec_default);
	}
}

void Config::parse_input(Module_Config::Input& i, libconfig::Setting& cfg){
	std::string str_source;
	cfg.lookupValue("source", str_source);
	// convert string to lowercase and evaluate source
	std::transform(str_source.begin(), str_source.end(), str_source.begin(), ::tolower);
	if(str_source == "pulse"){
		i.source = Module_Config::Source::PULSE;
	}else{
		i.source = Module_Config::Source::FIFO;
	}

	cfg.lookupValue("file", i.file);
	cfg.lookupValue("device", i.device);
	cfg.lookupValue("stereo", i.stereo);
	cfg.lookupValue("f_sample", i.f_sample);
}

void Config::parse_oscilloscope(Module_Config::Oscilloscope& o, libconfig::Setting& cfg){
	cfg.lookupValue("channel", o.channel);
	o.channel = std::min(o.channel, 1);
	cfg.lookupValue("scale", o.scale);
	cfg.lookupValue("width", o.width);
	cfg.lookupValue("sigma", o.sigma);
	cfg.lookupValue("sigma_coeff", o.sigma_coeff);

	parse_color(o.color, "color", cfg);
	parse_transformation(o.pos, "pos", cfg);
}

void Config::parse_spectrum(Module_Config::Spectrum& s, libconfig::Setting& cfg, const Module_Config::FFT& fft){
	cfg.lookupValue("channel", s.channel);
	s.channel = std::min(s.channel, 1);
	//cfg.lookupValue("output_size", output_size);
	s.scale = fft.scale;

	// calculate data buffer offset and length
	int f_start, f_stop;
	if(cfg.lookupValue("f_start", f_start) && cfg.lookupValue("f_stop", f_stop)){
		f_start = std::max(f_start, 0);
		f_stop = std::max(f_stop, 0);
		if(f_stop > f_start){
			int& data_offset = s.data_offset;
			int& output_size = s.output_size;
			data_offset = std::floor((float) f_start / fft.d_freq);
			output_size = std::ceil((float) f_stop / fft.d_freq) - (data_offset - 1);
			output_size = std::min(output_size, (int)fft.size - data_offset);
		}
	}

	// log frequency settings
	cfg.lookupValue("log_start", s.log_start);
	cfg.lookupValue("log_enabled", s.log_enabled);

	cfg.lookupValue("min_db", s.min_db);
	cfg.lookupValue("max_db", s.max_db);

	if(s.max_db > s.min_db){
		float max_n = s.max_db * 0.05;
		float min_n = s.min_db * 0.05;
		s.slope = -2.0 / (min_n - max_n);
		s.offset = 1.0 - s.slope * max_n;
	}

	parse_color(s.top_color, "top_color", cfg);
	parse_color(s.bot_color, "bot_color", cfg);
	parse_color(s.line_color, "line_color", cfg);
	parse_transformation(s.pos, "pos", cfg);


	cfg.lookupValue("gradient", s.gradient);
	cfg.lookupValue("bar_width", s.bar_width);
	cfg.lookupValue("gravity", s.gravity);

	try{
		libconfig::Setting& rb_setting = cfg.lookup("rainbow");
		rb_setting.lookupValue("enabled", s.rainbow);
		if(s.rainbow){
			parse_rainbow(s, rb_setting);
		}
	}
	catch(const libconfig::SettingNotFoundException& e){}

	cfg.lookupValue("dB_lines", s.dB_lines);
}

void Config::parse_rainbow(Module_Config::Spectrum& s, libconfig::Setting& cfg){
	try{
		libconfig::Setting& rb_phase = cfg.lookup("phase");
		rb_phase.lookupValue("r", s.phase_d.rgba[0]);
		rb_phase.lookupValue("g", s.phase_d.rgba[1]);
		rb_phase.lookupValue("b", s.phase_d.rgba[2]);
		s.bot_color = s.phase_d;
	
		libconfig::Setting& rb_freq = cfg.lookup("freq");
		rb_freq.lookupValue("r", s.freq_d.rgba[0]);
		rb_freq.lookupValue("g", s.freq_d.rgba[1]);
		rb_freq.lookupValue("b", s.freq_d.rgba[2]);
		for(int i = 0; i < 3; i++){
			s.top_color.rgba[i] = s.phase_d.rgba[i] + s.freq_d.rgba[i];
		}
		s.top_color.rgba[3] = 1;
	}
	catch(const libconfig::SettingNotFoundException& e){}
}

void Config::parse_color(Module_Config::Color& c,const std::string& path, libconfig::Setting& cfg){
	std::string color;
	cfg.lookupValue(path, color);
	if(color == ""){
		return;
	}
	try{
		if(color[0] == '#'){
			color = color.substr(1);
		}
		long value = std::stol(color, nullptr, 16);
		if(color.length() == 8){
		// color with alpha value
			// normalize alpha
			c.rgba[3] = static_cast<float>((value / 0x1000000) % 0x100) / 255.;
			c.rgba[0] = static_cast<float>((value / 0x10000) % 0x100);
			c.rgba[1] = static_cast<float>((value / 0x100) % 0x100);
			c.rgba[2] = static_cast<float>(value % 0x100);
			c.normalize();
		}else{
		// opaque color
			// calculate rbg bytes
			c.rgba[0] = static_cast<float>((value / 0x10000) % 0x100);
			c.rgba[1] = static_cast<float>((value / 0x100) % 0x100);
			c.rgba[2] = static_cast<float>(value % 0x100);
			c.rgba[3] = 1;
			c.normalize();
		}
	}catch(std::invalid_argument& e){
		std::cerr << "Unable to parse color of setting: " << cfg.getPath() << path << std::endl;
	}catch(std::out_of_range& e){
		// ignore empty strings
	}
}

void Config::parse_transformation(Module_Config::Transformation& t, const std::string& path, libconfig::Setting& cfg){
	try{
		libconfig::Setting& sett = cfg.lookup(path);

		sett.lookupValue("xmin", t.Xmin);
		sett.lookupValue("xmax", t.Xmax);
		sett.lookupValue("ymin", t.Ymin);
		sett.lookupValue("ymax", t.Ymax);
	}
	catch(const libconfig::SettingNotFoundException& e){}
}
