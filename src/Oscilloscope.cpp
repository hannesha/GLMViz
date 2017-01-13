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

#include "Oscilloscope.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

Oscilloscope::Oscilloscope(Config& config, const unsigned o_id): id(o_id){
	init_crt();

	configure(config);

	//update_x_buffer(config.buf_size);
}

void Oscilloscope::draw(){
	sh_crt.use();
	v_crt.bind();

	glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, size);
}

void Oscilloscope::init_crt(){
	const char* vert_code = 
	"#version 150\n"
	"in float y;"
	"in float x;"
	"uniform float scale;"
	"void main(){"
	"  gl_Position = vec4(x, clamp(y * scale, -1.0, 1.0), 0.0, 1.0);"
	"}";

	GL::Shader vert(vert_code, GL_VERTEX_SHADER);

	const char* geom_code =
	#include "shader/smooth_lines.geom"
	;

	GL::Shader geom(geom_code, GL_GEOMETRY_SHADER);

	const char* frag_code =
	#include "shader/simple.frag"
	;

	GL::Shader frag(frag_code, GL_FRAGMENT_SHADER);

	sh_crt.link({vert, geom, frag});

	v_crt.bind();

	b_crt_y.bind();
	GLint arg_y = sh_crt.get_attrib("y");
	glVertexAttribPointer(arg_y, 1, GL_SHORT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_y);

	b_crt_x.bind();
	GLint arg_x = sh_crt.get_attrib("x");
	glVertexAttribPointer(arg_x, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_x);

	GL::VAO::unbind();
}

void Oscilloscope::configure(Config& config){
	Config::Oscilloscope ocfg = config.oscilloscopes[id];
	sh_crt();

	GLint i_scale = sh_crt.get_uniform("scale");
	glUniform1f(i_scale, ocfg.scale/32768.0);

	GLint i_color = sh_crt.get_uniform("line_color");
	glUniform4fv(i_color, 1, ocfg.color.rgba);

	GLint i_width = sh_crt.get_uniform("width");
	glUniform1f(i_width, ocfg.width);

	set_transformation(ocfg.pos);

	channel = std::min(ocfg.channel, 1);
}

void Oscilloscope::update_x_buffer(const size_t size){
	std::vector<float> x_data(size);
	float f_size = (float)size;

	// generate x positions ranging from -1 to 1
	for(unsigned int i = 0; i < size; i++){
		x_data[i] = (((float) i + 0.5) - (f_size * 0.5)) / (f_size * 0.5);
	}

	b_crt_x.bind();
	glBufferData(GL_ARRAY_BUFFER, x_data.size() * sizeof(float), &x_data[0], GL_STATIC_DRAW);
}

void Oscilloscope::set_transformation(Config::Transformation& t){
	glm::mat4 transformation = glm::ortho(t.Xmin, t.Xmax, t.Ymin, t.Ymax);

	sh_crt();

	GLint i_trans = sh_crt.get_uniform("trans");
	glUniformMatrix4fv(i_trans, 1, GL_FALSE, glm::value_ptr(transformation)); 
}

void Oscilloscope::update_buffer(Buffer<int16_t>& buffer){
	auto lock = buffer.lock();
	// resize x coordinate buffer if necessary
	if(size != buffer.size){
		size = buffer.size;
		update_x_buffer(size);

		b_crt_y.bind();
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(int16_t), &buffer.v_buffer[0], GL_DYNAMIC_DRAW);
	}else{
		b_crt_y.bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(int16_t), &buffer.v_buffer[0]);
	}

}

void Oscilloscope::update_buffer(Buffer<int16_t>& lbuffer, Buffer<int16_t>& rbuffer){
	if(channel == 0){
		update_buffer(lbuffer);
	}else{
		update_buffer(rbuffer);
	}
}
