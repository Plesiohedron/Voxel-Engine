#version 460 core

#extension GL_NV_gpu_shader5 : enable

layout (location = 0) in uint16_t position;
layout (location = 1) in uint16_t UV;

out vec2 fragUV;

uniform mat4 model;

void main() {

	float UVx = ((UV >> 5u) & 0x1Fu) / 16.0f;
	float UVy = (UV & 0x1Fu) / 16.0f;

	float x = ((position >> 10u) & 0x1Fu);
	float y = ((position >> 5u) & 0x1Fu);

	fragUV = vec2(UVx, UVy);
	gl_Position = model * vec4(x, y, 0.0f, 1.0f);
}