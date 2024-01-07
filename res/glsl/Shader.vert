#version 460 core

#extension GL_NV_gpu_shader5 : enable

layout (location = 0) in uint16_t color;
layout (location = 1) in uint16_t UV;
layout (location = 2) in uint16_t position;

//out vec4 fragBrithness;
out vec2 fragUV;
//out vec3 fragPosition;
//out vec4 fragColor;

uniform mat4 projection;
uniform mat4 view;
//uniform mat4 projview;
uniform mat4 model;

void main() {

	float r = ((color >> 12u) & 0xFu) / 15.0f;
	float g = ((color >> 8u) & 0xFu) / 15.0f;
	float b = ((color >> 4u) & 0xFu) / 15.0f;
	float a = (color & 0xFu) / 15.0f;

	float UVx = ((UV >> 5u) & 0x1Fu) / 16.0f;
	float UVy = (UV & 0x1Fu) / 16.0f;

	float x = ((position >> 10u) & 0x1Fu);
	float y = ((position >> 5u) & 0x1Fu);
	float z = (position & 0x1Fu);

	//fragColor = vec4(r, g, b, a);
	fragUV = vec2(UVx - 0.0000001f, UVy - 0.0000001f);
	gl_Position = projection * view * model * vec4(x, y, z, 1.0f);
}