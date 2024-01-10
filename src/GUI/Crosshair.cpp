#include "Crosshair.h"

Crosshair::Crosshair(int width, int height) {
    crosshairShader.bindAttribute(0, "position");
    crosshairShader.bindAttribute(1, "UV");
    crosshairShader.link();

    uniformCrosshairTextureLoc = crosshairShader.getUniformLocation("CrosshairTexture");
    uniformCrosshairRegionTextureLoc = crosshairShader.getUniformLocation("CrosshairRegionTexture");
    uniformModelLoc = crosshairShader.getUniformLocation("model");

    crosshairTexture.setImage(Image::LoadImage("icons.png"));

    crosshairRegionTexture.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    crosshairVAO.bind();
    crosshairVAO.initializeVBO(vertices, 8);
    crosshairVAO.initializeEBO(indexes, 6);
    crosshairVAO.postInitialization();

    updateModel(width, height);
}

void Crosshair::makeCrosshairRegionTexture() {
    glReadPixels(windowWidth / 2 - crosshairWidth / 2, windowHeight / 2 - crosshairHeight / 2, crosshairWidth, crosshairHeight, GL_RGB, GL_UNSIGNED_BYTE, crosshairRegionColors);
    crosshairRegionTexture.bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, crosshairWidth, crosshairHeight, GL_RGB, GL_UNSIGNED_BYTE, crosshairRegionColors);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Crosshair::updateModel(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-static_cast<float>(crosshairWidth) / windowWidth, -static_cast<float>(crosshairHeight) / windowHeight, 0.0f));
    model = glm::scale(model, glm::vec3(static_cast<float>(crosshairWidth) / (windowWidth / 2), static_cast<float>(crosshairHeight) / (windowHeight / 2), 1.0f));
}

void Crosshair::draw(bool cursorIsMoving, bool cursorIsLocked) {
    if (cursorIsMoving && cursorIsLocked)
        makeCrosshairRegionTexture();

    crosshairShader.use();
    crosshairShader.uniformMatrix(uniformModelLoc, model);

    glActiveTexture(GL_TEXTURE0);
    crosshairTexture.bind();
    crosshairShader.uniformTexture(uniformCrosshairTextureLoc, 0);

    glActiveTexture(GL_TEXTURE1);
    crosshairRegionTexture.bind();
    crosshairShader.uniformTexture(uniformCrosshairRegionTextureLoc, 1);

    crosshairVAO.draw(GL_TRIANGLES, 6);
}
