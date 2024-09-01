#version 460 core

in vec2 frag_coordinates;
in vec3 frag_UV;

flat in vec3 color0;
flat in vec3 color1;
flat in vec3 color2;
flat in vec3 color3;
flat in uint texture_id;

out vec4 color;

uniform sampler2DArray texture0;

void main() {
    if (texture_id == 3) {
        color = vec4(mix(mix(color3, color0, frag_coordinates.y), mix(color2, color1, frag_coordinates.y), frag_coordinates.x) * vec3(0.5566f, 0.8721f, 0.3255f), 1.0f) * texture(texture0, frag_UV) +
                vec4(mix(mix(color3, color0, frag_coordinates.y), mix(color2, color1, frag_coordinates.y), frag_coordinates.x), 1.0f) * texture(texture0, vec3(frag_UV.x, frag_UV.y, frag_UV.z - 1));
    } else {
        color = vec4(mix(mix(color3, color0, frag_coordinates.y), mix(color2, color1, frag_coordinates.y), frag_coordinates.x), 1.0f) * texture(texture0, frag_UV);
    }
}