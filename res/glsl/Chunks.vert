#version 460 core

layout (location = 0) in uvec2 data;

layout (std430, binding = 0) buffer Matrices {
    mat4 models[];
};

out vec3 frag_UV;
out vec4 frag_color;

uniform mat4 projection;
uniform mat4 view;
uniform int model_index;

void main() {
	float AO = float((data.x >> 30u) & 3u);

	uint UV_layer = uint(data.y & 0xFFu);
	uint UVx = uint((data.x >> 6u) & 0x3Fu);
	uint UVy = uint(data.x & 0x3Fu);

	float r = (float((data.y >> 20u) & 0xFu) / 15.0f) * (1.0f - 0.2f * AO);
	float g = (float((data.y >> 16u) & 0xFu) / 15.0f) * (1.0f - 0.2f * AO);
	float b = (float((data.y >> 12u) & 0xFu) / 15.0f) * (1.0f - 0.2f * AO);
	float s = (float((data.y >> 8u) & 0xFu) / 15.0f);

	float x = float((data.x >> 24u) & 0x3Fu);
	float y = float((data.x >> 18u) & 0x3Fu);
	float z = float((data.x >> 12u) & 0x3Fu);

	frag_color = vec4(r, g, b, 1.0f);
	frag_UV = vec3(UVx, UVy, UV_layer);
	gl_Position = projection * view * models[model_index] * vec4(x, y, z, 1.0f);
}