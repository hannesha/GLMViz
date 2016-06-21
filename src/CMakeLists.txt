find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(Threads REQUIRED)
FIND_LIBRARY(FFTW3_LIBRARIES NAMES fftw3 libfftw3)
pkg_search_module(GLFW REQUIRED glfw3) 


include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${FFTW3_INCLUDE_DIRS})


add_executable(Viz viz_spec.cpp)


target_link_libraries(Viz ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${GLM_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${FFTW3_LIBRARIES})
