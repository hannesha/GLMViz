#include "Config.hpp"

Config::Config(){
	std::string file = "/.config/GLViz/config";
	file = std::string(getenv("HOME")) + file;
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
		
		// TODO: replace values with from/to dB range
		cfg.lookupValue("slope", slope);
		cfg.lookupValue("offset", offset);
		

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
