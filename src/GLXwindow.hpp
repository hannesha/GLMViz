#pragma once

#include <string>
#include <vector>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <GL/glx.h>

class GLXwindow {
	public:
		GLXwindow(const int, const int);
		~GLXwindow();

		// extension string helper
		bool hasExt(const std::string& exts, const std::string& ext){
			return exts.find(ext) != std::string::npos;
		};

		inline void swapBuffers(){ glXSwapBuffers(display, win); };
		void set_title(const std::string&);

		// resource wrappers
		template<typename T>
		class Xptr{
			public:
				using FN_del = int(*)(T*);

				Xptr(T* p, FN_del fn_del){ ptr = p; deleter = fn_del; };
				~Xptr(){ if(is_valid()) deleter(ptr); };
				Xptr(Xptr&) = delete;
				Xptr(Xptr&& p){ ptr = p.ptr; p.ptr = nullptr; deleter = p.deleter; };
				
				inline bool is_valid(){ return ptr != nullptr; };
				inline T* get(){ return ptr; };
				inline T& operator[](int i){ return ptr[i]; };
				inline T* operator->(){ return get(); };
				inline T* operator()(){ return get(); };

			private:
				T* ptr;
				FN_del deleter;
		};

		template<typename T, typename FN_resdel>
		class Res{
			public:
				Res(T res, FN_resdel fn_del, Display* disp){ this->res = res; deleter = fn_del; display = disp; };
				~Res(){ if(is_valid()) deleter(display, res); };
				Res(const Res&) = delete;
				Res(Res&& r): res(std::move(r.res)), display(std::move(r.display)){ deleter = r.deleter; };

				inline T get(){ return res; };
				inline T operator()(){ return get(); };
				inline bool is_valid(){ return res != 0; };

			private:
				T res;
				FN_resdel deleter;
				Display* display;
		};

		// delete function alias
		template<typename T>
		using FN_del = int(*)(T*);

		template<typename T>
		using FN_glxresdel = void(*)(Display*, T);
		template<typename T>
		using FN_xresdel = int(*)(Display*, T);

		template<typename T>
		using GLXres = Res<T, FN_glxresdel<T>>;

		template<typename T>
		using Xres = Res<T, FN_xresdel<T>>;
		
		Display* display;
		//Xptr<XVisualInfo> vi;
		Colormap cmap;
		Window win;
		GLXContext ctx;

	private:
		void selectFBConfig(GLXFBConfig&, Display*, int[]);

		// glXCreateContextAttribsARB function pointer alias
		using glXCreateContextAttribsARBProc = GLXContext (*)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
};
