deps = []
deps += dependency('gl')
deps += dependency('threads')
dep_fftw = dependency('fftw3f')
deps += dependency('libconfig++')
dep_glm = dependency('glm', required: false)
deps += dep_fftw
deps += dep_glm

if not dep_glm.found()
	message('Can\'t find glm pkg-config file, checking glm headers.')
	glm_header = meson.get_compiler('cpp').has_header('glm/glm.hpp')
	if glm_header
		message('glm found, consider updating your glm library.')
	else
		error('glm not found!')
	endif
endif

src = ['Buffer.cpp', 'Config.cpp', 'Config_Monitor.cpp', 'FFT.cpp', 'Fifo.cpp', 'GLMViz.cpp', 'Inotify.cpp', 'Oscilloscope.cpp', 'Program.cpp', 'Spectrum.cpp', 'xdg.cpp']
# simd optimization (for rms calculation)
add_project_arguments('-fopenmp-simd', language: 'cpp')

opt_pulse = dependency('libpulse', required: false)
opt_pulse_simple = dependency('libpulse-simple', required: false)

if opt_pulse.found() and opt_pulse_simple.found()
	src += 'Pulse.cpp'
	deps += [opt_pulse, opt_pulse_simple]
	add_project_arguments('-DWITH_PULSE', language : 'cpp')
endif

if get_option('transparency') 
	src += 'GLXwindow.cpp'
	deps += dependency('x11')
	add_project_arguments('-DWITH_TRANSPARENCY', language: 'cpp')
else
	deps += dependency('glfw3')
endif


exe = executable('glmviz', src, dependencies: deps, install: true)

fft_ex = executable('fft_example', ['FFT_example.cpp', 'FFT.cpp', 'Buffer.cpp'], dependencies: [dep_fftw])
