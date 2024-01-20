#version 460 core

in vec2 fragUV;

out vec4 color;

uniform sampler2D crosshairTexture;
uniform sampler2D crosshairRegionTexture;

void main() {
    vec4 crosshairTexturePixelColor = texture(crosshairTexture, fragUV);
    vec4 crosshairRegionTexturePixelColor = texture(crosshairRegionTexture, vec2(fragUV.x * 16, 1 - fragUV.y * 16));

    if (crosshairTexturePixelColor.r == 1.0 
        && crosshairTexturePixelColor.g == 1.0 
        && crosshairTexturePixelColor.b == 1.0 
        && crosshairTexturePixelColor.a == 1.0) {

        color = vec4(1.0f - crosshairRegionTexturePixelColor.rgb, 1.0f);
    } else 
        color = crosshairTexturePixelColor;

    //color = CrosshairTexturePixelColor;
}