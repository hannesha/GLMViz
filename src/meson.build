dep_gl = dependency('gl')
dep_threads = dependency('threads')
dep_fftw = dependency('fftw3f')
dep_glfw = dependency('glfw3')
dep_cfgpp = dependency('libconfig++')
dep_basedir = dependency('libxdg-basedir')

opt_pulse = dependency('libpulse', required: false)
opt_pulse_simple = dependency('libpulse-simple', required: false)

src_pulse = []
dep_pulse = []
if opt_pulse.found() and opt_pulse_simple.found()
	src_pulse = 'Pulse.cpp'
	dep_pulse = [opt_pulse, opt_pulse_simple]
	add_project_arguments('-DWITH_PULSE', language : 'cpp')
endif

src = ['GLMViz.cpp', 'Program.cpp', 'FFT.cpp', 'Spectrum.cpp', 'Oscilloscope.cpp', 'Fifo.cpp', 'Buffer.cpp', 'Config.cpp']

exe = executable('glmviz', [src, src_pulse], dependencies: [dep_gl, dep_threads, dep_fftw, dep_glfw, dep_cfgpp, dep_basedir, dep_pulse])
