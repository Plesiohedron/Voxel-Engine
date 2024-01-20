#version 460 core

//in vec4 fragBrithness;
in vec3 fragUV;
//in vec4 fragPosition;
in vec4 fragColor;

out vec4 color;

uniform sampler2DArray texture0;
//uniform vec3 CameraPosition;

void main() {
    color = fragColor * texture(texture0, vec3(fragUV));
}