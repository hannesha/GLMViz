R"(
#version 150

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;

out vec4 color;

uniform float width;
uniform vec4 line_color;
uniform mat4 trans;

// edge limits
const vec2 limit_min = vec2(-0.6,-1.0);
const vec2 limit_max = vec2( 0.6, 1.0);
void main () {
	color = line_color;
	vec2 p0 = gl_in[0].gl_Position.xy;
	vec2 p1 = gl_in[1].gl_Position.xy;
	vec2 p2 = gl_in[2].gl_Position.xy;
	vec2 p3 = gl_in[3].gl_Position.xy;

	// calculate tangents
	vec2 p01 = normalize(p1 - p0);
	vec2 p12 = normalize(p2 - p1);
	vec2 p23 = normalize(p3 - p2);

	vec2 t1 = normalize(p01 + p12);
	vec2 t2 = normalize(p12 + p23);

	// calculate miter, clamp hard edges
	vec2 m1 = clamp(vec2(-t1.y, t1.x), limit_min, limit_max) * width;
	vec2 m2 = clamp(vec2(-t2.y, t2.x), limit_min, limit_max) * width;

	// draw line vertices
	gl_Position = trans * vec4(p1 + m1, 0.0, 1.0);
	EmitVertex();

	gl_Position = trans * vec4(p2 + m2, 0.0, 1.0);
	EmitVertex();

	gl_Position = trans * vec4(p1 - m1, 0.0, 1.0);
	EmitVertex();

	gl_Position = trans * vec4(p2 - m2, 0.0, 1.0);
	EmitVertex();

	EndPrimitive();
}	
)"
