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

#include "GL_utils.hpp"

namespace GL{
	class Multisampler{
		public:
			Multisampler(const int nsamples, const int w, const int h): samples(nsamples){
				resize(samples, w, h);
				fbms.bind();
				if(samples > 0){
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex_fb.id, 0);
				}else{
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_fb.id, 0);
				}
				FBO::unbind();
				};
			
			inline void bind(){ fbms.bind(); };
			inline void blit(const int w1, const int h1, const int w2, const int h2){ 
				fbms(GL_READ_FRAMEBUFFER);
				FBO::unbind(GL_DRAW_FRAMEBUFFER);
				glBlitFramebuffer(0, 0, w1, h1, 0, 0, w2, h2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

				// bind default FBO
				FBO::unbind();
				};
			inline void blit(const int w, const int h){ blit(w, h, w, h); };

			void resize(const int nsamples, const int w, const int h){
				if(samples > 0){
					tex_fb.bind(GL_TEXTURE_2D_MULTISAMPLE);
					glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nsamples, GL_RGBA, w, h, GL_TRUE);
					Texture::unbind(GL_TEXTURE_2D_MULTISAMPLE);
				}else{
					tex_fb.bind(GL_TEXTURE_2D);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_BYTE, nullptr);
					Texture::unbind(GL_TEXTURE_2D);
				}
				};

			FBO fbms;
			Texture tex_fb;
			int samples;
	};
}
