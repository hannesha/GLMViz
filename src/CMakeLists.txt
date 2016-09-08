find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(Threads REQUIRED)
pkg_search_module(FFTW3 REQUIRED fftw3 libfftw3)
pkg_search_module(GLFW REQUIRED glfw3) 
pkg_search_module(CONFIG++ REQUIRED libconfig++)


include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${FFTW3_INCLUDE_DIRS})
include_directories(${CONFIG++_INCLUDE_DIRS})


add_executable(Viz GLViz.cpp Program.cpp Shader.cpp FFT.cpp Input.cpp Config.cpp)


target_link_libraries(Viz ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${FFTW3_LIBRARIES} ${CONFIG++_LIBRARIES})
