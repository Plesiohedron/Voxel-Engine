#version 460 core

//in vec4 fragBrithness;
in vec2 fragUV;
//in vec4 fragPosition;
in vec4 fragColor;

out vec4 color;

uniform sampler2D texture0;
//uniform vec3 CameraPosition;

void main() {
    color = texture(texture0, fragUV);
    //color = fragColor;
}