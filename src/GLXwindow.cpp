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

#include "GLXwindow.hpp"
#include <iostream>
#include <stdexcept>

void GLXwindow::selectFBConfig(GLXFBConfig& fbconfig, Display* display, int visual_attribs[]){
	int fbcount;
	Xptr<GLXFBConfig> fbconfigs(glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount), (FN_del<GLXFBConfig>)XFree);

	//std::cout << fbcount << " matching fbconfigs" << std::endl;
	int goodfb = -1;
	for(int i = 0; i<fbcount; i++){
		Xptr<XVisualInfo> vi(glXGetVisualFromFBConfig(display, fbconfigs[i]), (FN_del<XVisualInfo>)XFree);
		if(vi.is_valid()){
			XRenderPictFormat* pic = XRenderFindVisualFormat(display, vi->visual);

			if(pic->direct.alphaMask > 0){
				goodfb = i;
			}
		}
	}

	if(goodfb >= 0){
		// get matching fbconfig
		fbconfig = fbconfigs[goodfb];
	}else{
		// use default fbconfig config
		fbconfig = fbconfigs[0];
	}
}

void GLXwindow::set_title(const std::string& title){
	XStoreName(display, win, title.c_str());
}

GLXwindow::GLXwindow(const int width, const int height){
	display = XOpenDisplay(nullptr);
	if(display == nullptr){
		throw std::runtime_error("Can't open display!");
	}

	int glx_major, glx_minor;
	if(!glXQueryVersion(display, &glx_major, &glx_minor) || (glx_minor < 3)){
		XCloseDisplay(display);

		throw std::runtime_error("Outdated GLX version!");
	}

	std::string glx_exts = glXQueryExtensionsString(display, DefaultScreen(display));
	bool arb_create_context = hasExt(glx_exts, "GLX_ARB_create_context");
	if(!arb_create_context){
		XCloseDisplay(display);

		throw std::runtime_error("Can't create modern OpenGL context!");
	}

	int visual_attribs[] = {
		GLX_X_RENDERABLE,	True,
		GLX_RENDER_TYPE,	GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE,	GLX_TRUE_COLOR,
		GLX_BUFFER_SIZE,	32,
		GLX_RED_SIZE,		8,
		GLX_GREEN_SIZE,		8,
		GLX_BLUE_SIZE,		8,
		GLX_ALPHA_SIZE,		8,
		GLX_DOUBLEBUFFER,	True,
		None
	};

	// select fbconfig with RGBA support
	GLXFBConfig fbconfig;
	selectFBConfig(fbconfig, display, visual_attribs);

	// get visual info and create color map
	Xptr<XVisualInfo> vi(glXGetVisualFromFBConfig(display, fbconfig), (FN_del<XVisualInfo>)XFree);
	cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);

	XSetWindowAttributes wa;
	wa.colormap = cmap;
	wa.background_pixmap = None;
	wa.border_pixel = 0;
	wa.event_mask = ExposureMask;

	// open Xwindow
	win = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel|CWBackPixmap|CWColormap|CWEventMask, &wa);

	// load context creation function
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");

	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	ctx = glXCreateContextAttribsARB(display, fbconfig, 0, True, context_attribs);
	if(ctx == 0){
		XDestroyWindow(display, win);
		XFreeColormap(display, cmap);
		XCloseDisplay(display);

		throw std::runtime_error("Failed to create GL3.3 context!");
	}

	glXMakeCurrent(display, win, ctx);

	// show window
	XMapWindow(display, win);
};

GLXwindow::~GLXwindow(){
	glXMakeCurrent(display, 0, 0);
	glXDestroyContext(display, ctx);
	XDestroyWindow(display, win);
	XFreeColormap(display, cmap);
	XCloseDisplay(display);
}
