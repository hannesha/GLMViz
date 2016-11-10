/*
 *	GLMViz is a OpenGL based Visualizer for mpd.
 *	Copyright (C) 2016  Hannes Haberl
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

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <thread>

#include <iostream>

// Include basic GL utility headers
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include helper files
#include "Program.hpp"
#include "GL_utils.hpp"
#include "FFT.hpp"
#include "Input.hpp"
#include "Fifo.hpp"
#include "Buffer.hpp"
#include "Config.hpp"
#include "Spectrum.hpp"

#ifdef WITH_PULSE
#include "Pulse.hpp"
#endif
