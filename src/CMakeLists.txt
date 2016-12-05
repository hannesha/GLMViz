find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(Threads REQUIRED)
pkg_search_module(FFTW3 REQUIRED fftw3f libfftw3f)
pkg_search_module(GLFW REQUIRED glfw3) 
pkg_search_module(CONFIG++ REQUIRED libconfig++)
pkg_search_module(BASEDIR REQUIRED libxdg-basedir)

find_package(PulseAudio)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${FFTW3_INCLUDE_DIRS})
include_directories(${CONFIG++_INCLUDE_DIRS})
include_directories(${BASEDIR_INCLUDE_DIRS})

if(PULSEAUDIO_FOUND)
	Message("PulseAudio found. Building with PulseAudio support.")
	include_directories(${PULSEAUDIO_INCLUDE_DIRS})
	add_definitions(-DWITH_PULSE)
	set(PULSE_LIBS ${PULSE_LIBS} "pulse" "pulse-simple")
	set(PULSE_FILES "Pulse.cpp")
endif(PULSEAUDIO_FOUND)

add_executable(glmviz GLMViz.cpp Program.cpp FFT.cpp Spectrum.cpp Oscilloscope.cpp Fifo.cpp ${PULSE_FILES} Buffer.cpp Config.cpp)

target_link_libraries(glmviz ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${FFTW3_LIBRARIES} ${CONFIG++_LIBRARIES} ${BASEDIR_LIBRARIES} ${PULSE_LIBS})

# install GLMViz
install(TARGETS glmviz DESTINATION "/usr/local/bin/")
