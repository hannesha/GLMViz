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

#include "Spectrum.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

Spectrum::Spectrum(Config& config){
	init_bar_shader(sh_bars, config.rainbow);
	init_line_shader(sh_lines);
	init_bar_gravity_shader(sh_bars_pre);

	arg_y = sh_bars.get_attrib("y");
	arg_gravity_old = sh_bars_pre.get_attrib("gravity_old");
	arg_time_old = sh_bars_pre.get_attrib("time_old");

	set_transformation();
	set_uniforms(config);

	init_bars();
	init_bars_pre(config);
	init_lines();

	update_x_buffer(config.output_size);
}

void Spectrum::draw(const size_t size, const bool draw_lines){
	/* render lines */
	if(draw_lines){
		sh_lines.use();
		v_lines.bind();
		glDrawArrays(GL_LINES, 0, sizeof(dB_lines) / sizeof(float));
	}


	/* gravity processing shader */
	v_bars_pre.bind();
	b_fb2.bind();
	// set TF attribute pointers
	glVertexAttribPointer(arg_gravity_old, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glVertexAttribPointer(arg_time_old, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)(sizeof(float)));

	sh_bars_pre.use();
	// bind fist feedback buffer as TF buffer
	b_fb1.tfbind();

	// begin transform feedback
	glBeginTransformFeedback(GL_POINTS);
	glEnable(GL_RASTERIZER_DISCARD);

	glDrawArrays(GL_POINTS, 0, size);

	// disable TF
	glDisable(GL_RASTERIZER_DISCARD);
	glEndTransformFeedback();

	//undbind both feedback buffers and swap them
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
	std::swap(b_fb1.id, b_fb2.id);


	/* render bars */
	sh_bars.use();
	v_bars.bind();
	// use second feedback buffer for drawing
	b_fb2.bind();
	// reconfigure attribute pointer
	glVertexAttribPointer(arg_y, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)(2*sizeof(float)));
	glDrawArrays(GL_POINTS, 0, size);
}

void Spectrum::fill_tf_buffers(const size_t size){
	b_fb1.bind();
	glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(float), 0, GL_DYNAMIC_DRAW);

	b_fb2.bind();
	glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(float), 0, GL_DYNAMIC_DRAW);
}

void Spectrum::update_x_buffer(const size_t size){
	std::vector<float> x_data(size);
	float f_size = (float)size;

	// generate x positions ranging from -1 to 1
	for(unsigned int i = 0; i < size; i++){
		x_data[i] = (((float) i + 0.5) - (f_size * 0.5)) / (f_size * 0.5);
	}

	b_x.bind();
	glBufferData(GL_ARRAY_BUFFER, x_data.size() * sizeof(float), &x_data[0], GL_STATIC_DRAW);
}

void Spectrum::update_fft(FFT& fft, const size_t size){
	b_fft.bind();
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(fftwf_complex), fft.output, GL_DYNAMIC_DRAW);
}

void Spectrum::set_uniforms(Config& cfg){
	sh_bars.use();
	// Post compute specific uniforms
	GLint i_width = sh_bars.get_uniform("width");
	glUniform1f(i_width, cfg.bar_width/(float)cfg.output_size);

	// set bar color gradients
	GLint i_top_color = sh_bars.get_uniform("top_color");
	glUniform4fv(i_top_color, 1, cfg.top_color);

	GLint i_bot_color = sh_bars.get_uniform("bot_color");
	glUniform4fv(i_bot_color, 1, cfg.bot_color);

	GLint i_gradient = sh_bars.get_uniform("gradient");
	glUniform1f(i_gradient, cfg.gradient);


	sh_bars_pre.use();
	// set precompute shader uniforms
	GLint i_fft_scale = sh_bars_pre.get_uniform("fft_scale");
	glUniform1f(i_fft_scale, cfg.fft_scale);

	GLint i_slope = sh_bars_pre.get_uniform("slope");
	glUniform1f(i_slope, cfg.slope * 0.5);

	GLint i_offset = sh_bars_pre.get_uniform("offset");
	glUniform1f(i_offset, cfg.offset * 0.5);

	GLint i_gravity = sh_bars_pre.get_uniform("gravity");
	glUniform1f(i_gravity, cfg.gravity / cfg.fps);


	sh_lines.use();
	// set dB line specific arguments
	i_offset = sh_lines.get_uniform("offset");
	glUniform1f(i_offset, cfg.offset);

	i_slope = sh_lines.get_uniform("slope");
	glUniform1f(i_slope, cfg.slope);
	
	GLint i_line_color = sh_lines.get_uniform("line_color");
	glUniform4fv(i_line_color, 1, cfg.line_color);
}

void Spectrum::init_bars(){
	v_bars.bind();

	b_x.bind();
	// set x position data in bar VAO
	GLint arg_x_data = sh_bars.get_attrib("x");
	glVertexAttribPointer(arg_x_data, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_x_data);

	// enable the preprocessed y attribute
	GLint arg_y = sh_bars.get_attrib("y");
	glEnableVertexAttribArray(arg_y);
}

void Spectrum::init_bars_pre(Config& config){
	/* Pre compute shader */
	v_bars_pre.bind();
	
	// init ping-pong feedback buffers
	fill_tf_buffers(config.output_size);

	b_fft.bind();
	// set fft_data buffer as vec2 input for the shader
	GLint arg_fft_output = sh_bars_pre.get_attrib("a");
	glVertexAttribPointer(arg_fft_output, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_fft_output);

	// enable precompute shader attributes
	GLint arg_gravity_old = sh_bars_pre.get_attrib("gravity_old");
	glEnableVertexAttribArray(arg_gravity_old);

	GLint arg_time_old = sh_bars_pre.get_attrib("time_old");
	glEnableVertexAttribArray(arg_time_old);
}

void Spectrum::init_lines(){
	v_lines.bind();

	b_lines.bind();
	glBufferData(GL_ARRAY_BUFFER, sizeof(dB_lines), dB_lines, GL_STATIC_DRAW);

	GLint arg_line_vert = sh_lines.get_attrib("pos");
	glVertexAttribPointer(arg_line_vert, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(arg_line_vert);
}

void Spectrum::set_transformation(){
	// apply simple ortho transformation
	glm::mat4 transformation = glm::ortho(-1.0,1.0,-1.0,1.0,-1.0,1.0);

	sh_bars.use();
	GLint i_trans = sh_bars.get_uniform("trans");
	glUniformMatrix4fv(i_trans, 1, GL_FALSE, glm::value_ptr(transformation));

	sh_lines.use();
	i_trans = sh_lines.get_uniform("trans");
	glUniformMatrix4fv(i_trans, 1, GL_FALSE, glm::value_ptr(transformation));
}
