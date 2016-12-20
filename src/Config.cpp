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

#include "Config.hpp"

#include <basedir_fs.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cmath>

Config::Config(){
	xdgHandle xdghandle;

	if(xdgInitHandle(&xdghandle)){
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

		std::string str_source;
		cfg.lookupValue("source", str_source);
		// convert string to lowercase and evaluate source
		std::transform(str_source.begin(), str_source.end(), str_source.begin(), ::tolower);
		if(str_source == "pulse"){
			source = Source::PULSE;
		}else{
			source = Source::FIFO;
		}

		cfg.lookupValue("fifo_file", fifo_file);
		cfg.lookupValue("stereo", stereo);

		cfg.lookupValue("duration", duration);
		cfg.lookupValue("FS", FS);
		cfg.lookupValue("fps", fps);
		cfg.lookupValue("fft_size", fft_size);

		buf_size = FS * duration / 1000;
		fft_output_size = fft_size/2+1;
		d_freq = (float) FS / (float) fft_size;


		// normalization value for the fft output
		fft_scale = 1.0f/((float)(buf_size/2+1)*32768.0f);

		osc_default.parse("Osc", cfg);
		for(unsigned i = 0; i < 4; i++){
			if(cfg.exists("Osc" + std::to_string(i+1))){
				// init new Oscilloscope with default parameters
				Oscilloscope tmp = osc_default;
				// parse Oscilloscope and add it to the list
				tmp.parse("Osc" + std::to_string(i+1), cfg);

				// reuse old values if possible
				if(oscilloscopes.size() > i){
					oscilloscopes[i] = tmp;
				}else{
					oscilloscopes.push_back(tmp);
				}
			}else{
				break;
			}
		}

		spec_default.parse("Spectrum", cfg);
		spec_default.clamp_output_size(fft_output_size);
		for(unsigned i = 0; i < 4; i++){
			if(cfg.exists("Spectrum" + std::to_string(i+1))){
				// init new Spectrum with default parameters
				Spectrum tmp = spec_default;
				// parse Spectrum and add it to the list
				tmp.parse("Spectrum" + std::to_string(i+1), cfg);
				tmp.clamp_output_size(fft_output_size);

				// reuse old values if possible
				if(spectra.size() > i){
					spectra[i] = tmp;
				}else{
					spectra.push_back(tmp);
				}
			}else{
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

void Config::Oscilloscope::parse(const std::string& path, libconfig::Config& cfg){
	cfg.lookupValue(path + ".channel", channel);
	cfg.lookupValue(path + ".scale", scale);

	color.parse(path + ".color", cfg);
	//color.normalize(c);

	pos.parse(path + ".pos", cfg);
}

void Config::Spectrum::parse(const std::string& path, libconfig::Config& cfg){
	cfg.lookupValue(path + ".channel", channel);
	cfg.lookupValue(path + ".output_size", output_size);

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
	cfg.lookupValue(path + ".gravity", gravity);
	cfg.lookupValue(path + ".bar_width", bar_width);

	cfg.lookupValue(path + ".rainbow.enabled", rainbow);
	if(rainbow){
		parse_rainbow(path + ".rainbow", cfg);
	}

	cfg.lookupValue(path + ".dB_lines", dB_lines);

}

void Config::Spectrum::parse_rainbow(const std::string& path, libconfig::Config& cfg){
	Color freq_d = {1, 1, 1, 1};
	Color phase_d = {0, 0, 0, 1};

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
	try{	
		// remove # character
		size_t pos = 0;
		if(color.at(0) == '#') pos = 1;
		int value = std::stoi(color, &pos, 16);
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
