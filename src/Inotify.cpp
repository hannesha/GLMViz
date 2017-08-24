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

#include "Inotify.hpp"
//#include <memory>
#include <cstring>
#include <stdexcept>
#include <unistd.h>

inline std::string err_str(int& errnum){
	int err = errnum;
	errnum = 0;
	return std::string(std::strerror(err));
}

template<typename K>
Inotify<K>::Inotify(){
	instance_fd = inotify_init();
	if(instance_fd < 0){
		throw std::runtime_error("inotify init failed!, err: " + ::err_str(errno));
	}
}

template<typename K>
Inotify<K>::~Inotify(){
	// remove all watches
	for(auto& watch : wds){
		inotify_rm_watch(instance_fd, watch.second);
	}
	// destroy instance
	if(instance_fd >= 0) close(instance_fd);
}

template<typename K>
void Inotify<K>::add_watch(K key, const std::string& file, uint32_t mask){
	int wd = inotify_add_watch(instance_fd, file.c_str(), mask);
	if(wd >= 0){
		wds.emplace(key, wd);
	}else{
		throw std::runtime_error("Can't add watch, err: " + ::err_str(errno));
	}
}

template<typename K>
void Inotify<K>::rm_watch(K key){
	int rc = inotify_rm_watch(instance_fd, wds.at(key));
	if(rc != 0){
		wds.erase(key);
		errno = 0;
		//throw std::runtime_error("Can't remove watch, err: " + ::err_str(errno));
	}
}
