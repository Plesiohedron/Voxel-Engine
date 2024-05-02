#version 460 core

#extension GL_NV_gpu_shader5 : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UV;

out vec2 frag_UV;

uniform mat4 model;

void main() {
	frag_UV = UV;
	gl_Position = model * vec4(position, 1.0f);
}