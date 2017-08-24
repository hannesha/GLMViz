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
