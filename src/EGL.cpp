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

#include "EGL.hpp"
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

namespace EGL{
	// extension string helper
	bool hasExt(const std::string& exts, const std::string& ext){
		return exts.find(ext) != std::string::npos;
	}

	Surface::Surface(EGLSurface s, EGLDisplay d) : display(d){
		if(s == EGL_NO_SURFACE){
			throw std::runtime_error("Can't create EGLsurface");
		}
		surface = s;
	}

	Context::Context(EGLNativeDisplayType w_display): display(eglGetDisplay(w_display)){
		if(display.get() == EGL_NO_DISPLAY){
			throw std::runtime_error("Can't get EGL Display");
		}

		int major, minor;
		if(eglInitialize(display.get(), &major, &minor) != EGL_TRUE){
			throw std::runtime_error("Can't init EGL");
		}

		if(major <= 1 && minor < 4){
			throw std::runtime_error("Outdated EGL version");
		}

		if(eglBindAPI(EGL_OPENGL_API) != EGL_TRUE){
			throw std::runtime_error("Can't set API to OPENGL!");
		}

		int n_configs;
		eglGetConfigs(display.get(), nullptr, 0, &n_configs);
		std::cout << n_configs << std::endl;

		std::vector<EGLConfig> configs(n_configs);

		EGLint config_attribs[] = {
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
				EGL_RED_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_BLUE_SIZE, 8,
				EGL_ALPHA_SIZE, 8,
				EGL_NONE
		};

		eglChooseConfig(display.get(), config_attribs, configs.data(), n_configs, &n_configs);
		if(n_configs <= 0){
			throw std::runtime_error("Can't get framebuffer configuration");
		}
		configs.resize(n_configs);
		std::cout << " got configs: " << n_configs << std::endl;

		fbconfig = configs[0];
		for (auto& config : configs){
			int id, sample_depth, alpha, samples;
			eglGetConfigAttrib(display.get(), config, EGL_ALPHA_SIZE, &alpha);
			eglGetConfigAttrib(display.get(), config, EGL_CONFIG_ID, &id);
			eglGetConfigAttrib(display.get(), config, EGL_DEPTH_SIZE, &sample_depth);
			eglGetConfigAttrib(display.get(), config, EGL_SAMPLES, &samples);

//			std::cout << "ID: " << id << " alpha: " << alpha << " sample_depth: " << sample_depth << " samples: "
//					  << samples
//					  << std::endl;

			if(samples == 4){
				fbconfig = config;
				break;
			}
		}

		// load KRH create context
		std::string egl_exts = eglQueryString(display.get(), EGL_EXTENSIONS);
		if(!hasExt(egl_exts, "EGL_KHR_create_context")){
			throw std::runtime_error("Ext not found!");
		}

		// set opengl version to 3.3 core
		int context_attribs[] = {
				EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
				EGL_CONTEXT_MINOR_VERSION_KHR, 3,
				EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
				EGL_NONE
		};

		context = eglCreateContext(display.get(), fbconfig, EGL_NO_CONTEXT, context_attribs);
		if(context == EGL_NO_CONTEXT){
			throw std::runtime_error("Context creation failed!");
		}

	}
}