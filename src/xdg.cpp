/*
 *	Copyright (C) 2017  Hannes Haberl
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

#include "xdg.hpp"

#include <pwd.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstdlib>

namespace xdg{
	std::string config_home(){
		std::string config_home;
		const char* cxdg_cfg_home = std::getenv("XDG_CONFIG_HOME");
		if(cxdg_cfg_home != nullptr){
			config_home = cxdg_cfg_home;
		}

		return config_home;
	}

	std::string default_config_home(){
		std::string config_home;
		// get default config directory
		struct passwd* pw = ::getpwuid(::getuid());
		config_home = pw->pw_dir;
		config_home += "/.config";

		return config_home;
	}

	std::vector<std::string> config_dirs(){
		std::string config_dirs;
		const char* cxdg_cfg_dirs = std::getenv("XDG_CONFIG_DIRS");
		if(cxdg_cfg_dirs != nullptr){
			config_dirs = cxdg_cfg_dirs;
		}

		std::vector<std::string> dirs;
		std::istringstream ss(config_dirs);
		std::string dir;

		while(std::getline(ss, dir, ':')){
			dirs.push_back(dir);
		}

		return dirs;
	}
	
	std::string default_config_dir(){ return "/etc/xdg"; }

	bool verify_path(const std::string& path){
		std::ifstream file(path);
		return file.good();
	}

	std::string find_config(const std::string& path){
		std::string config;
		// check XDG_CONFIG_HOME
		config = config_home() + path;
		if(verify_path(config)) return config;

		// use default value for XDG_CONFIG_HOME
		config = default_config_home() + path;
		if(verify_path(config)) return config;

		// check XDG_CONFIG_DIRS
		for(std::string& dir : config_dirs()){
			config = dir + path;
			if(verify_path(config)) return config;
		}

		// check XDG_CONFIG_DIRS default value
		config = default_config_dir() + path;
		if(verify_path(config)) return config;

		// return empty string if no config file was found
		return "";
	}
}
