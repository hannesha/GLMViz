R"(
#version 150

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 v_bot_color[];
in vec4 v_top_color[];

out vec4 color;

uniform float width;
uniform mat4 trans;

void main () {
	float x1 = gl_in[0].gl_Position.x - width;
	float x2 = gl_in[0].gl_Position.x + width;
	vec4 vwidth = vec4(width, 0.0, 0.0, 0.0);

	color = v_bot_color[0];
	gl_Position = trans * vec4(x1, -1.0, 0.0, 1.0);
	EmitVertex();

	color = v_top_color[0];
	gl_Position = trans * (gl_in[0].gl_Position - vwidth);
	EmitVertex();

	color = v_bot_color[0];
	gl_Position = trans * vec4(x2, -1.0, 0.0, 1.0);
	EmitVertex();

	color = v_top_color[0];
	gl_Position = trans * (gl_in[0].gl_Position + vwidth);
	EmitVertex();

	EndPrimitive();
}	
)"
