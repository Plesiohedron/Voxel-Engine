#version 460 core

layout (location = 0) in uvec4 light;
layout (location = 1) in uint dimension;

layout (std430, binding = 0) buffer Matrices {
    mat4 models[];
};

out vec2 frag_coordinates;
out vec3 frag_UV;

flat out vec3 color0;
flat out vec3 color1;
flat out vec3 color2;
flat out vec3 color3;
flat out uint texture_id;

uniform mat4 projection;
uniform mat4 view;
uniform int model_index;

void main() {
	uint UV_layer = uint((dimension >> 25u) & 0xFFu);
	uint UVx =      uint((dimension >> 5u)  & 0x1Fu);
	uint UVy =      uint(dimension          & 0x1Fu);

	float x = float((dimension >> 20u) & 0x1Fu);
	float y = float((dimension >> 15u) & 0x1Fu);
	float z = float((dimension >> 10u) & 0x1Fu);

	float r, g, b, s;

	r = (float((light.a >> 18u) & 0x3Fu) / 4.0f / 15.0f);
	g = (float((light.a >> 12u)  & 0x3Fu) / 4.0f / 15.0f);
	b = (float((light.a >> 6u) & 0x3Fu) / 4.0f / 15.0f);
	s = (float(light.a          & 0x3Fu) / 4.0f / 15.0f);

	color0 = vec3(r + s, g + s, b + s);
		
	r = (float((light.b >> 18u) & 0x3Fu) / 4.0f / 15.0f);
	g = (float((light.b >> 12u)  & 0x3Fu) / 4.0f / 15.0f);
	b = (float((light.b >> 6u)  & 0x3Fu) / 4.0f / 15.0f);
	s = (float(light.b          & 0x3Fu) / 4.0f / 15.0f);

	color1 = vec3(r + s, g + s, b + s);

	r = (float((light.g >> 18u) & 0x3Fu) / 4.0f / 15.0f);
	g = (float((light.g >> 12u)  & 0x3Fu) / 4.0f / 15.0f);
	b = (float((light.g >> 6u)  & 0x3Fu) / 4.0f / 15.0f);
	s = (float(light.g          & 0x3Fu) / 4.0f / 15.0f);

	color2 = vec3(r + s, g + s, b + s);

	r = (float((light.r >> 18u) & 0x3Fu) / 4.0f / 15.0f);
	g = (float((light.r >> 12u)  & 0x3Fu) / 4.0f / 15.0f);
	b = (float((light.r >> 6u)  & 0x3Fu) / 4.0f / 15.0f);
	s = (float(light.r          & 0x3Fu) / 4.0f / 15.0f);

	color3 = vec3(r + s, g + s, b + s);

	if (UV_layer == 4) {
		color0 *= vec3(0.5566f, 0.8721f, 0.3255f);
		color1 *= vec3(0.5566f, 0.8721f, 0.3255f);
		color2 *= vec3(0.5566f, 0.8721f, 0.3255f);
		color3 *= vec3(0.5566f, 0.8721f, 0.3255f);
	}
	texture_id = UV_layer;

	if (gl_VertexID % 4 == 0) {
		frag_coordinates = vec2(0, 0);
	} else if (gl_VertexID % 4 == 1) {
		frag_coordinates = vec2(1, 0);
	} else if (gl_VertexID % 4 == 2) {
		frag_coordinates = vec2(1, 1);
	} else {
		frag_coordinates = vec2(0, 1);
	}

	frag_UV = vec3(UVx, UVy, UV_layer);
	gl_Position = projection * view * models[model_index] * vec4(x, y, z, 1.0f);
}