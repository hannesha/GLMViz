R"(
#version 150

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

out vec4 color;
out vec4 t;

uniform float width;
uniform float sigma;
uniform float sigma_coeff;
uniform vec4 line_color;
uniform mat4 trans;

void main () {
	color = line_color;
	vec2 p0 = gl_in[0].gl_Position.xy;
	vec2 p1 = gl_in[1].gl_Position.xy;

	// calculate tangents
	vec2 len = vec2(sigma, length(p1 - p0)*sigma_coeff*sigma);
	vec2 p01 = normalize(p1 - p0);

	// calculate miter
	vec2 m1 = vec2(-p01.y, p01.x) * width;

	// draw line vertices
	t = vec4(1., -1., len);
	gl_Position = trans * vec4(p0 + m1, 0.0, 1.0);
	EmitVertex();

	t = vec4(1., 1., len);
	gl_Position = trans * vec4(p1 + m1, 0.0, 1.0);
	EmitVertex();

	t = vec4(-1., -1., len);
	gl_Position = trans * vec4(p0 - m1, 0.0, 1.0);
	EmitVertex();

	t = vec4(-1., 1., len);
	gl_Position = trans * vec4(p1 - m1, 0.0, 1.0);
	EmitVertex();

	EndPrimitive();
}
)"
