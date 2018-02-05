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
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <type_traits>
#include <memory>

namespace EGL{
#define MAKE_DELETER(NAME, FN) \
    struct NAME{ \
        template<typename T> void operator()(T* ptr){ FN(ptr); } \
    }

	MAKE_DELETER(Display_deleter, eglTerminate);
#undef MAKE_DELETER
	using Display = std::unique_ptr<std::remove_pointer<EGLDisplay>::type, Display_deleter>;

	class Surface{
	public:
		Surface(EGLSurface, EGLDisplay);

		~Surface(){
			if(surface != EGL_NO_SURFACE){
				eglDestroySurface(display, surface);
			}
		}

		Surface(Surface&) = delete;
		Surface(Surface&& s) noexcept {
			surface = s.surface;
			display = s.display;

			s.surface = EGL_NO_SURFACE;
		}
		Surface& operator=(Surface&&) = default;

		void swap_buffers(){
			eglSwapBuffers(display, surface);
		}

		inline EGLSurface get(){
			return surface;
		}

	private:
		EGLSurface surface;
		EGLDisplay display;
	};

	class Context{
	public:
		explicit Context(EGLNativeDisplayType w_display);

		~Context(){
			//eglMakeCurrent(display(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroyContext(display.get(), context);
		}

		inline int makeCurrent(EGLSurface a, EGLSurface b){
			return eglMakeCurrent(display.get(), a, b, context);
		}

		inline int makeCurrent(Surface& a, Surface& b){
			return eglMakeCurrent(display.get(), a.get(), b.get(), context);
		}

		Surface make_surface(NativeWindowType w){
			Surface s(eglCreateWindowSurface(display.get(), fbconfig, w, nullptr), display.get());
			return s;
		}

	private:
		Display display;
		EGLContext context;
		EGLConfig fbconfig;
	};
}