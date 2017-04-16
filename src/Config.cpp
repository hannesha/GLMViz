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

#include <basedir_fs.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cmath>

Config::Config(const std::string& config_file){
	file = config_file;

	xdgHandle xdghandle;

	if(file == "" && xdgInitHandle(&xdghandle)){
                file = xdgConfigFind("GLMViz/config", &xdghandle);

                xdgWipeHandle(&xdghandle);
	}

	if(file == "") file = "/etc/GLMViz/config";

	reload();
}

void Config::reload(){
	try{
		cfg.readFile(file.c_str());

		cfg.lookupValue("Window.AA", w_aa);
		cfg.lookupValue("Window.height", w_height);
		cfg.lookupValue("Window.width", w_width);

		input.parse("Input", cfg);

		cfg.lookupValue("duration", duration);
		cfg.lookupValue("fps", fps);
		cfg.lookupValue("fft_size", fft_size);

		buf_size = input.f_sample * duration / 1000;
		fft_output_size = fft_size/2+1;
		d_freq = (float) input.f_sample / (float) fft_size;


		// normalization value for the fft output
		fft_scale = 1.0f/((float)(buf_size/2+1)*32768.0f);

		osc_default.parse("Osc", cfg);
		for(unsigned i = 0; i < MAX_OSCILLOSCOPES; i++){
			if(cfg.exists("Osc" + std::to_string(i+1))){
				// init new Oscilloscope with default parameters
				Oscilloscope tmp = osc_default;
				// parse Oscilloscope and add it to the list
				tmp.parse("Osc" + std::to_string(i+1), cfg);

				// reuse old values if possible
				try{
					oscilloscopes.at(i) = tmp;
				}catch(std::out_of_range& e){
					oscilloscopes.push_back(tmp);
				}
			}else{
				// delete remaining oscilloscopes
				if(oscilloscopes.size() > i){
					oscilloscopes.erase(oscilloscopes.begin() + i, oscilloscopes.end());
				}
				break;
			}
		}

		spec_default.parse("Spectrum", cfg, fft_output_size, fft_scale, fps);
		for(unsigned i = 0; i < MAX_SPECTRA; i++){
			if(cfg.exists("Spectrum" + std::to_string(i+1))){
				// init new Spectrum with default parameters
				Spectrum tmp = spec_default;
				// parse Spectrum and add it to the list
				tmp.parse("Spectrum" + std::to_string(i+1), cfg, fft_output_size, fft_scale, fps);

				// reuse old values if possible
				try{
					spectra.at(i) = tmp;
				}catch(std::out_of_range& e){
					spectra.push_back(tmp);
				}
			}else{
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
}

void Config::Input::parse(const std::string& path, libconfig::Config& cfg){
	std::string str_source;
	cfg.lookupValue(path + ".source", str_source);
	// convert string to lowercase and evaluate source
	std::transform(str_source.begin(), str_source.end(), str_source.begin(), ::tolower);
	if(str_source == "pulse"){
		source = Source::PULSE;
	}else{
		source = Source::FIFO;
	}

	cfg.lookupValue(path + ".file", file);
	cfg.lookupValue(path + ".device", device);
	cfg.lookupValue(path + ".stereo", stereo);
	cfg.lookupValue(path + ".f_sample", f_sample);
}

void Config::Oscilloscope::parse(const std::string& path, libconfig::Config& cfg){
	cfg.lookupValue(path + ".channel", channel);
	channel = std::min(channel, 1);
	cfg.lookupValue(path + ".scale", scale);
	cfg.lookupValue(path + ".width", width);

	color.parse(path + ".color", cfg);

	pos.parse(path + ".pos", cfg);
}

void Config::Spectrum::parse(const std::string& path, libconfig::Config& cfg, size_t fft_size, float fft_scale, int fps){
	cfg.lookupValue(path + ".channel", channel);
	channel = std::min(channel, 1);
	cfg.lookupValue(path + ".output_size", output_size);
	output_size = std::min(output_size, (int)fft_size);
	scale = fft_scale;

	cfg.lookupValue(path + ".min_db", min_db);
	cfg.lookupValue(path + ".max_db", max_db);

	if(max_db > min_db){
		float max_n = max_db * 0.05;
		float min_n = min_db * 0.05;
		slope = -2.0 / (min_n - max_n);
		offset = 1.0 - slope * max_n;
	}

	top_color.parse(path + ".top_color", cfg);
	bot_color.parse(path + ".bot_color", cfg);
	line_color.parse(path + ".line_color", cfg);

	pos.parse(path + ".pos", cfg);

	cfg.lookupValue(path + ".gradient", gradient);
	bool got_gravity = cfg.lookupValue(path + ".gravity", gravity);
	if(got_gravity){
		gravity = gravity / (float)(fps * fps);
	}
	cfg.lookupValue(path + ".bar_width", bar_width);

	cfg.lookupValue(path + ".rainbow.enabled", rainbow);
	if(rainbow){
		parse_rainbow(path + ".rainbow", cfg);
	}

	cfg.lookupValue(path + ".dB_lines", dB_lines);

}

void Config::Spectrum::parse_rainbow(const std::string& path, libconfig::Config& cfg){
	char w_rgb[] = "rgb";
	
	for(int i = 0; i < 3; i++){
		cfg.lookupValue(path + ".phase." + w_rgb[i], phase_d.rgba[i]);
	}

	bot_color = phase_d;
	
	for(int i = 0; i < 3; i++){
		cfg.lookupValue(path + ".freq." + w_rgb[i], freq_d.rgba[i]);
		top_color.rgba[i] = phase_d.rgba[i] + freq_d.rgba[i];
	}
	top_color.rgba[3] = 1;
}

void Config::Color::parse(const std::string& path, libconfig::Config& cfg){
	std::string color;
	cfg.lookupValue(path, color);
	if(color == ""){
		return;
	}
	try{	
		int value = std::stoi(color, nullptr, 16);
		// calculate rbg bytes
		rgba[0] = static_cast<float>((value / 0x10000) % 0x100);
		rgba[1] = static_cast<float>((value / 0x100) % 0x100);	
		rgba[2] = static_cast<float>(value % 0x100);
		rgba[3] = 1;
		normalize();
	}catch(std::invalid_argument& e){
		std::cerr << "Unable to parse color of setting: " << path << std::endl;
	}catch(std::out_of_range& e){
		// ignore empty strings
	}
}

void Config::Color::normalize(const Color& c){
	// convert screen color(CRT gamma) to sRGB
	const float inv_gamma = 1.0/2.2;

	for(int i = 0; i < 3; i ++){
		rgba[i] = std::pow(c.rgba[i] / 255, inv_gamma);
	}
	/*r = std::pow(c.r / 255, inv_gamma); 
	g = std::pow(c.g / 255, inv_gamma);
	b = std::pow(c.b / 255, inv_gamma);
	a = c.a;*/
}

void Config::Color::normalize(){
	// convert screen color(CRT gamma) to sRGB
	const float inv_gamma = 1.0/2.2;
	for(int i = 0; i < 3; i ++){
		rgba[i] = std::pow(rgba[i] / 255, inv_gamma);
	}
	//r = std::pow(r / 255, inv_gamma); 
	//g = std::pow(g / 255, inv_gamma);
	//b = std::pow(b / 255, inv_gamma);
	//a = a;
}

void Config::Transformation::parse(const std::string& path, libconfig::Config& cfg){
	cfg.lookupValue(path + ".xmin", Xmin);
	cfg.lookupValue(path + ".xmax", Xmax);
	cfg.lookupValue(path + ".ymin", Ymin);
	cfg.lookupValue(path + ".ymax", Ymax);
}
