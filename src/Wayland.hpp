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

#pragma once
#include <wayland-client.h>
#include <wayland-egl.h>
#include <memory>
//#include "xdg-shell-client-protocol.h"

namespace Wayland{

// Delete Functor helper macro
#define MAKE_DELETER(NAME, FN) \
    struct NAME{ \
        template<typename T> void operator()(T* ptr){ FN(ptr); } \
    }

	MAKE_DELETER(Display_deleter, wl_display_disconnect);
	MAKE_DELETER(Registry_deleter, wl_registry_destroy);
	MAKE_DELETER(Surface_deleter, wl_surface_destroy);
	MAKE_DELETER(Shell_surface_deleter, wl_shell_surface_destroy);
	MAKE_DELETER(Callback_deleter, wl_callback_destroy);

//	MAKE_DELETER(XDG_shell_surface_deleter, xdg_surface_destroy);
//	MAKE_DELETER(XDG_toplevel_deleter, xdg_toplevel_destroy);

#undef MAKE_DELETER

	using Display = std::unique_ptr<wl_display, Display_deleter>;
	using Registry = std::unique_ptr<wl_registry, Registry_deleter>;
	using Surface = std::unique_ptr<wl_surface, Surface_deleter>;
	using Shell_surface = std::unique_ptr<wl_shell_surface, Shell_surface_deleter>;
	using Callback = std::unique_ptr<wl_callback, Callback_deleter>;

//	using XDG_Shell_surface = std::unique_ptr<xdg_surface, XDG_shell_surface_deleter>;
//	using XDG_Toplevel = std::unique_ptr<xdg_toplevel, XDG_toplevel_deleter>;

	class Client{
	public:
		Client();

		inline long dispatch(){
			return wl_display_dispatch(display.get());
		}

		inline long dispatch_pending(){
			return wl_display_dispatch_pending(display.get());
		}

		struct Userdata{
			wl_shell* shell = nullptr;
			wl_compositor* compositor = nullptr;
//			xdg_wm_base* xdg_shell = nullptr;


			//Userdata() = default;

//			~Userdata(){
//				if(shell){
//					wl_shell_destroy(shell);
//				}
//				if(compositor){
//					wl_compositor_destroy(compositor);
//				}
//			}

			bool have_all(){
				return shell != nullptr && compositor != nullptr;
			}
		};

		Userdata resources;

		Display display;
		Registry registry;

	private:
		static void reg_handler(void*, wl_registry*, uint32_t, const char*, uint32_t);
		static void reg_remove_handler(void*, wl_registry*, uint32_t);

//		static void xdg_shell_ping(void*, xdg_wm_base *, uint32_t);

	};

	class EGL_window{
	public:
		EGL_window(Surface&, int w, int h);

		~EGL_window(){
			wl_egl_window_destroy(handle);
		}

		wl_egl_window* get(){ return handle; };

		int width, height;
		wl_egl_window* handle;
	};
}