dep_gl = dependency('gl')
dep_threads = dependency('threads')
dep_fftw = dependency('fftw3f')
dep_glfw = dependency('glfw3')
dep_cfgpp = dependency('libconfig++')
dep_glm = dependency('glm', required: false)

if not dep_glm.found()
	message('Can\'t find glm pkg-config file, checking glm headers.')
	glm_header = meson.get_compiler('cpp').has_header('glm/glm.hpp')
	if glm_header
		message('glm found, consider updating your glm library.')
	else
		error('glm not found!')
	endif
endif

opt_pulse = dependency('libpulse', required: false)
opt_pulse_simple = dependency('libpulse-simple', required: false)

src_pulse = []
dep_pulse = []
if opt_pulse.found() and opt_pulse_simple.found()
	src_pulse = 'Pulse.cpp'
	dep_pulse = [opt_pulse, opt_pulse_simple]
	add_project_arguments('-DWITH_PULSE', language : 'cpp')
endif

src = ['GLMViz.cpp', 'Program.cpp', 'FFT.cpp', 'Spectrum.cpp', 'Oscilloscope.cpp', 'Fifo.cpp', 'Buffer.cpp', 'Config.cpp', 'xdg.cpp']

exe = executable('glmviz', [src, src_pulse], dependencies: [dep_gl, dep_threads, dep_fftw, dep_glfw, dep_cfgpp, dep_pulse], install: true)

fft_ex = executable('fft_example', ['FFT_example.cpp', 'FFT.cpp', 'Buffer.cpp'], dependencies: [dep_fftw])
