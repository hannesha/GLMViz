/*
 *	Copyright (C) 2018 Hannes Haberl
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

#include "Wayland.hpp"
#include <cstring>
#include <iostream>

// resource wrappers
namespace Wayland{

	inline bool equal(const char* a, const char* b){
		return std::strcmp(a, b) == 0;
	}


	Client::Client() : display(wl_display_connect(nullptr)){
		if(!display){
			throw std::runtime_error("Can't connect to display");
		}

		registry.reset(wl_display_get_registry(display.get()));
		if(!registry){
			throw std::runtime_error("Can't get registry");
		}

		const wl_registry_listener reg_listeners = {
				reg_handler,
				reg_remove_handler
		};

		wl_registry_add_listener(registry.get(), &reg_listeners, &resources);
		wl_display_roundtrip(display.get());

		if(!resources.have_all()){
			throw std::runtime_error("Can't get required resources");
		}

	}

	void Client::reg_handler(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version){
		// cast userdata
		auto* userdata = reinterpret_cast<Userdata*>(data);
		std::cout << "interface: " << interface << " version: " << version << std::endl;
		if(equal(interface, "wl_compositor")){
			userdata->compositor = (wl_compositor*) wl_registry_bind(registry, name, &wl_compositor_interface, 3);
			std::cout << "Found compositor!" << std::endl;
		}else if(equal(interface, "wl_shell")){
			userdata->shell = (wl_shell*) wl_registry_bind(registry, name, &wl_shell_interface, 1);
			std::cout << "Found shell!" << std::endl;
		}
//		else if(equal(interface, "xdg_shell")){
//			const xdg_wm_base_listener xdg_listener = {
//					xdg_shell_ping
//			};
//			userdata->xdg_shell = (xdg_wm_base*) wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
//
//			xdg_wm_base_add_listener(userdata->xdg_shell, &xdg_listener, nullptr);
//			std::cout << "Found xdg-shell!" << std::endl;
//		}
	}

//	void Client::xdg_shell_ping(void* data, xdg_wm_base* shell, uint32_t serial){
//		xdg_wm_base_pong(shell, serial);
//	};

	void Client::reg_remove_handler(void* data, wl_registry* registry, uint32_t name){
		std::cout << "Removed: " << name << std::endl;
	}


	EGL_window::EGL_window(Surface& s, int w, int h) : width(w), height(h){
			if(width < 0){
				width = 1;
			}
			if(height < 0){
				height = 1;
			}

			handle = wl_egl_window_create(s.get(), height, width);
			if(!handle){
				throw std::runtime_error("Can't create egl window");
			}
		}

}