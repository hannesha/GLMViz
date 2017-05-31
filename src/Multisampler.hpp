#pragma once

#include "GL_utils.hpp"

namespace GL{
	class Multisampler{
		public:
			Multisampler(const int samples, const int w, const int h){
				resize(samples, w, h);
				fbms.bind();
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex_fb.id, 0);
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

			void resize(const int samples, const int w, const int h){
				tex_fb.bind(GL_TEXTURE_2D_MULTISAMPLE);
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, w, h, GL_TRUE);
				Texture::unbind(GL_TEXTURE_2D_MULTISAMPLE);
				};

			FBO fbms;
			Texture tex_fb;
	};
}
