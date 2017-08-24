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

#pragma once

#include <map>
#include <string>
#include <sys/inotify.h>

template<typename K>
class Inotify{
	public:
		Inotify();
		~Inotify();

		void add_watch(K, const std::string&, uint32_t mask);
		void rm_watch(K);
		inline bool match(int wd, K key){
			return wd == wds.at(key);
		}

		inline int get_fd(){
			return instance_fd;
		}

		std::map<K, int> wds;
	private:
		int instance_fd;
};

template class Inotify<int>;
template class Inotify<char>;
