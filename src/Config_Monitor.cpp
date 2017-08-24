#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <poll.h>
#include "Inotify.hpp"
#include "Config_Monitor.hpp"

static const int timeout = 100;
void monitor(const std::string& file, std::atomic<bool>& running, std::atomic<bool>& changed){
	try{
		Inotify<char> inotify;
		inotify.add_watch(0, file, IN_MODIFY | IN_IGNORED);
		int fd = inotify.get_fd();

		while(running){
			struct pollfd pfd;
			pfd.fd = fd;
			pfd.events = POLLIN;

			int ret = poll(&pfd, 1, timeout);
			if(ret > 0){
				struct inotify_event e;
				read(fd, reinterpret_cast<char*>(&e), sizeof(struct inotify_event));
				if(inotify.match(e.wd, 0)){
					if(e.mask == IN_IGNORED){
						// delete ignored watch descriptor
						inotify.rm_watch(0);
						// make new watch descriptor on same file
						inotify.add_watch(0, file, IN_MODIFY | IN_IGNORED);
						//std::cout << "Vim edit" << std::endl;
					}
					//std::cout << "Edited" << std::endl;
					changed = true;
				}
			}
		}
	}catch(std::runtime_error& e){
		std::cerr << e.what() << std::endl;
	}
}
