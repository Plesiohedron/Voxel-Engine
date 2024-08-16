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
	uint UV_layer = uint((data.y >> 21u) & 0xFFu);
	uint UVx = uint((data.x >> 5u) & 0x1Fu);
	uint UVy = uint(data.x         & 0x1Fu);

	float r = (float((data.y >> 14u) & 0x7Fu) / 5.0f / 15.0f);
	float g = (float((data.y >> 7u)  & 0x7Fu) / 5.0f / 15.0f);
	float b = (float(data.y          & 0x7Fu) / 5.0f / 15.0f);
	float s = (float((data.x >> 25u) & 0x7Fu) / 5.0f / 15.0f);

	float x = float((data.x >> 20u) & 0x1Fu);
	float y = float((data.x >> 15u) & 0x1Fu);
	float z = float((data.x >> 10u) & 0x1Fu);

	if (UV_layer == 4) {
		frag_color = vec4(r + s, g + s, b + s, 1.0f) * vec4(0.5566f, 0.8721f, 0.3255f, 1.0f);
	} else {
		frag_color = vec4(r + s, g + s, b + s, 1.0f);
	}

	frag_UV = vec3(UVx, UVy, UV_layer);
	gl_Position = projection * view * models[model_index] * vec4(x, y, z, 1.0f);
}