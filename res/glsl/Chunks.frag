#version 460 core

in vec2 frag_coordinates;
in vec3 frag_UV;

flat in vec3 color0;
flat in vec3 color1;
flat in vec3 color2;
flat in vec3 color3;

out vec4 color;

uniform sampler2DArray texture0;

void main() {
    color = vec4(mix(mix(color3, color0, frag_coordinates.y), mix(color2, color1, frag_coordinates.y), frag_coordinates.x), 1.0f) * texture(texture0, frag_UV);
}