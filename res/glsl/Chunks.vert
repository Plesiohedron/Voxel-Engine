#version 460 core

layout (location = 0) in uvec2 data;

out vec3 frag_UV;
out vec4 frag_color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const vec2 texture_coordinates[4] = vec2[4](
	vec2(0.0f, 0.0f),
	vec2(1.0f, 0.0f),
	vec2(0.0f, 1.0f),
	vec2(1.0f, 1.0f)
);

void main() {
	float AO = float((data.x >> 24u) & 3u);

	uint UV_layer = uint(data.x & 0xFFu);
	uint tex_coords = uint((data.y >> 4u) & 3u);
	int w = int(data.y & 0xFu) + 1;
	int h = int((data.x >> 26u) & 0xFu) + 1;
	vec2 UV = texture_coordinates[tex_coords] * vec2(w, h);

	float r = (float((data.x >> 20u) & 0xFu) / 15.0f) * (1.0f - 0.2f * AO);
	float g = (float((data.x >> 16u) & 0xFu) / 15.0f) * (1.0f - 0.2f * AO);
	float b = (float((data.x >> 12u) & 0xFu) / 15.0f) * (1.0f - 0.2f * AO);
	float s = (float((data.x >> 8u) & 0xFu) / 15.0f);

	float x = float((data.y >> 16u) & 0x1Fu);
	float y = float((data.y >> 11u) & 0x1Fu);
	float z = float((data.y >> 6u) & 0x1Fu);

	frag_color = vec4(r, g, b, 1.0f);
	frag_UV = vec3(UV, UV_layer);
	gl_Position = projection * view * model * vec4(x, y, z, 1.0f);
}