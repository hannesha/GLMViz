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

Config::Config(){
	std::string file;
	xdgHandle xdghandle;

	if(xdgInitHandle(&xdghandle)){
                file = xdgConfigFind("GLMViz/config", &xdghandle);

                xdgWipeHandle(&xdghandle);
        }

	if(file == "") file = "/etc/GLMViz/config";

	try{
                cfg.readFile(file.c_str());

		cfg.lookupValue("Window.AA", w_aa);
		cfg.lookupValue("Window.height", w_height);
		cfg.lookupValue("Window.width", w_width);

		cfg.lookupValue("duration", duration);
		cfg.lookupValue("FS", FS);
		cfg.lookupValue("fps", fps);
		cfg.lookupValue("fft_size", fft_size);
		cfg.lookupValue("output_size", output_size);

		buf_size = FS * duration / 1000;
		fft_output_size = fft_size/2+1;
		d_freq = (float) FS / (float) fft_output_size;
		
		// normalization value for the fft output
		fft_scale = 1.0f/((float)(buf_size/2+1)*32768.0f);
		
		cfg.lookupValue("min_db", min_db);
		cfg.lookupValue("max_db", max_db);

		if(max_db > min_db){
			float max_n = max_db * 0.05;
			float min_n = min_db * 0.05;
			slope = -2.0f / (min_n - max_n);
			offset = 1.0f - slope * max_n;
		}
		
		std::string color_path = "top_color";
		read_rgba(color_path, top_color);
		color_path = "botcolor";
		read_rgba(color_path, bot_color);
		color_path = "line_color";
		read_rgba(color_path, line_color);

		cfg.lookupValue("gradient", gradient);
		cfg.lookupValue("gravity", gravity);
		cfg.lookupValue("rainbow", rainbow);
		cfg.lookupValue("bar_width", bar_width);

        }catch(const libconfig::FileIOException &fioex){
                std::cerr << "I/O error while reading file." << std::endl;

                std::cout << "Using default settings." << std::endl;
        }catch(const libconfig::ParseException &pex){
                std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                << " - " << pex.getError() << std::endl;

                std::cout << "Using default settings." << std::endl;
        }	
}

void Config::read_rgba(const std::string &path, float rgba[]){
	cfg.lookupValue(path + ".r", rgba[0]);
	cfg.lookupValue(path + ".g", rgba[1]);
	cfg.lookupValue(path + ".b", rgba[2]);
	cfg.lookupValue(path + ".a", rgba[3]);
}
