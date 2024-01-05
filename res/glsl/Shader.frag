#version 330 core

in vec4 fragBrithness;
in vec2 fragUV;
//in vec4 fragPosition;
//in vec3 fragColor;

out vec4 color;

uniform sampler2D texture0;
//uniform vec3 CameraPosition;

void main() {
    color = fragBrithness * texture(texture0, fragUV);
}