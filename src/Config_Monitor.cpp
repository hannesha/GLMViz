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

#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <poll.h>
#include "Inotify.hpp"
#include "Config_Monitor.hpp"

static const int timeout = 100;
void monitor(const std::string& file, std::atomic<bool>& running, std::atomic<bool>& changed){
	try{
		Inotify inotify;
		int wd = inotify.add_watch(file, IN_MODIFY | IN_IGNORED);
		int fd = inotify.get_fd();

		while(running){
			struct pollfd pfd;
			pfd.fd = fd;
			pfd.events = POLLIN;

			int ret = poll(&pfd, 1, timeout);
			if(ret > 0){
				struct inotify_event e;
				read(fd, reinterpret_cast<char*>(&e), sizeof(struct inotify_event));
				if(e.wd == wd){
					// vim edit workaround
					if(e.mask == IN_IGNORED){
						// delete ignored watch descriptor
						inotify.rm_watch(wd);
						// make new watch descriptor on same file
						wd = inotify.add_watch(file, IN_MODIFY | IN_IGNORED);
					}
					changed = true;
				}
			}
		}
	}catch(std::runtime_error& e){
		std::cerr << e.what() << std::endl;
	}
}
