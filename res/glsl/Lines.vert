#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

out vec4 frag_color;

uniform mat4 projection;
uniform mat4 view;

void main() {
	gl_Position = projection * view * vec4(position, 1);

	frag_color = color;
}