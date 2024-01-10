#version 460 core

in vec2 fragUV;

out vec4 color;

uniform sampler2D CrosshairTexture;
uniform sampler2D CrosshairRegionTexture;

void main() {
    vec4 CrosshairTexturePixelColor = texture(CrosshairTexture, fragUV);
    vec4 CrosshairRegionTexturePixelColor = texture(CrosshairRegionTexture, vec2(fragUV.x * 16, 1 - fragUV.y * 16));

    if (CrosshairTexturePixelColor.r == 1.0 
        && CrosshairTexturePixelColor.g == 1.0 
        && CrosshairTexturePixelColor.b == 1.0 
        && CrosshairTexturePixelColor.a == 1.0) {

        color = vec4(1.0f - CrosshairRegionTexturePixelColor.rgb, 1.0f);
    } else 
        color = CrosshairTexturePixelColor;

    //color = CrosshairTexturePixelColor;
}