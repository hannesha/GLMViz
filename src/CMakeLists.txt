include(CheckIncludeFileCXX)
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
#find_package(glm REQUIRED)
find_package(Threads REQUIRED)
pkg_search_module(GLM QUIET glm)
pkg_search_module(FFTW3 REQUIRED fftw3f libfftw3f)
pkg_search_module(CONFIG++ REQUIRED libconfig++)

find_package(PulseAudio)

option(glx "Build with glx frontend" ON)
option(wayland "Build with wayland frontend" OFF)
if(wayland)
	Message("Building with wayland frontend")
	add_definitions(-DWITH_WAYLAND)
	set(WIN_SRC Wayland.cpp EGL.cpp)

	pkg_search_module(EGL REQUIRED egl)
	pkg_search_module(WAYLAND REQUIRED wayland-client)
	pkg_search_module(WAYLAND_EGL REQUIRED wayland-egl)
	set(WIN_LIBS ${EGL_LIBRARIES} ${WAYLAND_LIBRARIES} ${WAYLAND_EGL_LIBRARIES})
elseif(glx)
	Message("Building with glx frontend")
	add_definitions(-DWITH_GLX)
	set(WIN_SRC GLXwindow.cpp)

	pkg_search_module(X11 REQUIRED x11)
	set(WIN_LIBS ${X11_LIBRARIES})
else(wayland)
	pkg_search_module(GLFW REQUIRED glfw3)
	set(WIN_LIBS ${GLFW_LIBRARIES})
endif(wayland)

#include_directories(${OPENGL_INCLUDE_DIRS})
#include_directories(${GLFW_INCLUDE_DIRS})
#include_directories(${FFTW3_INCLUDE_DIRS})
#include_directories(${CONFIG++_INCLUDE_DIRS})

if(NOT GLM_FOUND)
	CHECK_INCLUDE_FILE_CXX("glm/glm.hpp" HAVE_GLM_HPP)
	if(HAVE_GLM_HPP)
		Message("glm found, consider updating your glm library.")
	else(HAVE_GLM_HPP)
		Message(FATAL_ERROR "glm not found!")
	endif(HAVE_GLM_HPP)
endif(NOT GLM_FOUND)

if(PULSEAUDIO_FOUND)
	Message("PulseAudio found. Building with PulseAudio support.")
	include_directories(${PULSEAUDIO_INCLUDE_DIRS})
	add_definitions(-DWITH_PULSE)
	set(PULSE_LIBS ${PULSE_LIBS} "pulse")
	set(PULSE_FILES "Pulse_Async.cpp")
endif(PULSEAUDIO_FOUND)

add_executable(glmviz GLMViz.cpp Program.cpp FFT.cpp Spectrum.cpp Oscilloscope.cpp Fifo.cpp ${PULSE_FILES} Buffer.cpp Config.cpp Config_Monitor.cpp Inotify.cpp xdg.cpp ${WIN_SRC})

target_link_libraries(glmviz ${OPENGL_gl_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${FFTW3_LIBRARIES} ${CONFIG++_LIBRARIES} ${PULSE_LIBS} ${WIN_LIBS})

# fft test program
add_executable(fft_example FFT_example.cpp FFT.cpp Buffer.cpp)
target_link_libraries(fft_example ${FFTW3_LIBRARIES})

# install GLMViz
install(TARGETS glmviz DESTINATION ${CMAKE_INSTALL_PREFIX})
