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
			std::cerr << "The specified config file doesn't exist, falling back to default config!" << std::endl;
		}
	}

	if(file == "") file = xdg::find_config("/GLMViz/config");

	if(file == "") file = "/etc/GLMViz/config";

	reload();
}

void Config::reload(){
	try{
		cfg.readFile(file.c_str());

		cfg.lookupValue("Window.AA", w_aa);
		cfg.lookupValue("Window.height", w_height);
		cfg.lookupValue("Window.width", w_width);

		try{
			input.parse(cfg.lookup("Input"));
		}catch(const libconfig::SettingNotFoundException& e){}

		cfg.lookupValue("duration", duration);
		cfg.lookupValue("fps", fps);
		cfg.lookupValue("fft_size", fft.size);

		buf_size = input.f_sample * duration / 1000;
		fft.output_size = fft.size/2+1;
		fft.d_freq = (float) input.f_sample / (float) fft.size;

		bg_color.parse("bg_color", cfg.getRoot());

		// normalization value for the fft output
		// calculate effective fft input data size
		float isize = std::min(buf_size, fft.size)/2+1;
		fft.scale = 1.0f/(isize*32768.0f);

		try{
			osc_default.parse(cfg.lookup("Osc"));
		}
		catch(const libconfig::SettingNotFoundException& e){}

		for(unsigned i = 0; i < MAX_OSCILLOSCOPES; i++){
			std::string path = "Osc" + std::to_string(i+1);
			try{
				// init new Oscilloscope with default parameters
				Oscilloscope tmp = osc_default;
				// parse Oscilloscope and add it to the list
				tmp.parse(cfg.lookup(path));

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
			spec_default.parse(cfg.lookup("Spectrum"), fft, fps);
		}
		catch(const libconfig::SettingNotFoundException& e){}

		for(unsigned i = 0; i < MAX_SPECTRA; i++){
			std::string path = "Spectrum" + std::to_string(i+1);
			try{
				// init new Spectrum with default parameters
				Spectrum tmp = spec_default;
				// parse Spectrum and add it to the list
				tmp.parse(cfg.lookup(path), fft, fps);

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

void Config::Input::parse(libconfig::Setting& cfg){
	std::string str_source;
	cfg.lookupValue("source", str_source);
	// convert string to lowercase and evaluate source
	std::transform(str_source.begin(), str_source.end(), str_source.begin(), ::tolower);
	if(str_source == "pulse"){
		source = Source::PULSE;
	}else{
		source = Source::FIFO;
	}

	cfg.lookupValue("file", file);
	cfg.lookupValue("device", device);
	cfg.lookupValue("stereo", stereo);
	cfg.lookupValue("f_sample", f_sample);
}

void Config::Oscilloscope::parse(libconfig::Setting& cfg){
	cfg.lookupValue("channel", channel);
	channel = std::min(channel, 1);
	cfg.lookupValue("scale", scale);
	cfg.lookupValue("width", width);
	cfg.lookupValue("sigma", sigma);
	cfg.lookupValue("sigma_coeff", sigma_coeff);

	color.parse("color", cfg);
	pos.parse("pos", cfg);
}

void Config::Spectrum::parse(libconfig::Setting& cfg, const FFT& fft, const int fps){
	cfg.lookupValue("channel", channel);
	channel = std::min(channel, 1);
	//cfg.lookupValue("output_size", output_size);
	scale = fft.scale;

	// calculate data buffer offset and length
	int f_start, f_stop;
	if(cfg.lookupValue("f_start", f_start) && cfg.lookupValue("f_stop", f_stop)){
		f_start = std::max(f_start, 0);
		f_stop = std::max(f_stop, 0);
		if(f_stop > f_start){
			data_offset = std::floor((float) f_start / fft.d_freq);
			output_size = std::ceil((float) f_stop / fft.d_freq) - (data_offset - 1);
			output_size = std::min(output_size, (int)fft.size - data_offset);
		}
	}

	// log frequency settings
	cfg.lookupValue("log_start", log_start);
	cfg.lookupValue("log_enabled", log_enabled);

	cfg.lookupValue("min_db", min_db);
	cfg.lookupValue("max_db", max_db);

	if(max_db > min_db){
		float max_n = max_db * 0.05;
		float min_n = min_db * 0.05;
		slope = -2.0 / (min_n - max_n);
		offset = 1.0 - slope * max_n;
	}

	top_color.parse("top_color", cfg);
	bot_color.parse("bot_color", cfg);
	line_color.parse("line_color", cfg);
	pos.parse("pos", cfg);


	cfg.lookupValue("gradient", gradient);
	cfg.lookupValue("bar_width", bar_width);
	bool have_gravity = cfg.lookupValue("gravity", gravity);
	if(have_gravity){
		gravity = gravity / (float)(fps * fps);
	}

	try{
		libconfig::Setting& rb_setting = cfg.lookup("rainbow");
		rb_setting.lookupValue("enabled", rainbow);
		if(rainbow){
			parse_rainbow(rb_setting);
		}
	}
	catch(const libconfig::SettingNotFoundException& e){}

	cfg.lookupValue("dB_lines", dB_lines);
}

void Config::Spectrum::parse_rainbow(libconfig::Setting& cfg){
	try{
		libconfig::Setting& rb_phase = cfg.lookup("phase");
		rb_phase.lookupValue("r", phase_d.rgba[0]);
		rb_phase.lookupValue("g", phase_d.rgba[1]);
		rb_phase.lookupValue("b", phase_d.rgba[2]);
	}
	catch(const libconfig::SettingNotFoundException& e){}
	bot_color = phase_d;
	
	try{
		libconfig::Setting& rb_freq = cfg.lookup("freq");
		rb_freq.lookupValue("r", freq_d.rgba[0]);
		rb_freq.lookupValue("g", freq_d.rgba[1]);
		rb_freq.lookupValue("b", freq_d.rgba[2]);
		for(int i = 0; i < 3; i++){
			top_color.rgba[i] = phase_d.rgba[i] + freq_d.rgba[i];
		}
	}
	catch(const libconfig::SettingNotFoundException& e){}
	top_color.rgba[3] = 1;
}

void Config::Color::parse(const std::string& path, libconfig::Setting& cfg){
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
			rgba[3] = static_cast<float>((value / 0x1000000) % 0x100) / 255.;
			rgba[0] = static_cast<float>((value / 0x10000) % 0x100);
			rgba[1] = static_cast<float>((value / 0x100) % 0x100);
			rgba[2] = static_cast<float>(value % 0x100);
			normalize();
		}else{
		// opaque color
			// calculate rbg bytes
			rgba[0] = static_cast<float>((value / 0x10000) % 0x100);
			rgba[1] = static_cast<float>((value / 0x100) % 0x100);
			rgba[2] = static_cast<float>(value % 0x100);
			rgba[3] = 1;
			normalize();
		}
	}catch(std::invalid_argument& e){
		std::cerr << "Unable to parse color of setting: " << cfg.getPath() << path << std::endl;
	}catch(std::out_of_range& e){
		// ignore empty strings
	}
}

void Config::Color::normalize(const Color& c){
	std::copy(c.rgba, c.rgba + 4, rgba);
	normalize();
}

void Config::Color::normalize(){
	// convert screen color(CRT gamma) to sRGB
	for(int i = 0; i < 3; i ++){
		rgba[i] = rgba[i] / 255;
	}
}

void Config::Transformation::parse(const std::string& path, libconfig::Setting& cfg){
	try{
		libconfig::Setting& sett = cfg.lookup(path);

		sett.lookupValue("xmin", Xmin);
		sett.lookupValue("xmax", Xmax);
		sett.lookupValue("ymin", Ymin);
		sett.lookupValue("ymax", Ymax);
	}
	catch(const libconfig::SettingNotFoundException& e){}
}
