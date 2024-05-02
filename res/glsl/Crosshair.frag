#version 460 core

in vec2 frag_UV;

out vec4 color;

uniform sampler2D crosshair_texture;
uniform sampler2D crosshair_region_texture;

void main() {
    vec4 crosshair_texture_pixel_color = texture(crosshair_texture, frag_UV);

    if (crosshair_texture_pixel_color.r == 1.0 
        && crosshair_texture_pixel_color.g == 1.0 
        && crosshair_texture_pixel_color.b == 1.0 
        && crosshair_texture_pixel_color.a == 1.0) {

        vec4 crosshair_region_texture_pixel_color = texture(crosshair_region_texture, vec2(frag_UV.x * 16, 1 - frag_UV.y * 16));
        color = vec4(1.0f - crosshair_region_texture_pixel_color.rgb, 1.0f);
    } else 
        color = crosshair_texture_pixel_color;
}