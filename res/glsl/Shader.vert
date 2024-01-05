#version 330 core

layout (location = 0) in float brithness;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec3 position;
//layout (location = 3) in vec3 color;

out vec4 fragBrithness;
out vec2 fragUV;
out vec3 fragPosition;
//out vec3 fragColor;

uniform mat4 projection;
uniform mat4 view;
//uniform mat4 projview;
uniform mat4 model;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);
	//gl_Position = projview * model * vec4(position, 1.0f);
	//gl_Position = vec4(position, 1.0f);

	fragBrithness = vec4(brithness, brithness, brithness, 1.0f);
	fragUV = UV - vec2(0.0000001f, 0.0000001f);
	//FragPosition = (View * Model * vec4(Position, 1.0f)).xyz;
	//fragColor = color;
}