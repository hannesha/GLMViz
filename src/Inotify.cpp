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
