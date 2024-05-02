#version 460 core

#extension GL_NV_gpu_shader5 : enable

layout (location = 0) in uint16_t position;
layout (location = 1) in uint16_t UV;
layout (location = 2) in uint16_t color;

out vec3 frag_UV;
out vec4 frag_color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	float r = ((color >> 12u) & 0xFu) / 15.0f;
	float g = ((color >> 8u) & 0xFu) / 15.0f;
	float b = ((color >> 4u) & 0xFu) / 15.0f;
	float a = (color & 0xFu) / 15.0f;

	uint UV_layer = ((UV >> 10u) & 0x2Fu);
	float UVx = ((UV >> 5u) & 0x1Fu);
	float UVy = (UV & 0x1Fu);

	float x = ((position >> 10u) & 0x1Fu);
	float y = ((position >> 5u) & 0x1Fu);
	float z = (position & 0x1Fu);

	frag_color = vec4(r, g, b, a);
	frag_UV = vec3(UVx, UVy, UV_layer);
	gl_Position = projection * view * model * vec4(x, y, z, 1.0f);
}