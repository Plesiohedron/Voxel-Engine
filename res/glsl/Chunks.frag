#version 460 core

in vec3 frag_UV;
in vec4 frag_color;

out vec4 color;

uniform sampler2DArray texture0;

void main() {
    color = frag_color * texture(texture0, vec3(frag_UV));
}