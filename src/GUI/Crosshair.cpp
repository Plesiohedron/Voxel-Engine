#include "Crosshair.h"

const GLsizei Crosshair::crosshairWidth = 32;
const GLsizei Crosshair::crosshairHeight = 32;

unsigned Crosshair::windowWidth;
unsigned Crosshair::windowHeight;

glm::mat4 Crosshair::model(1.0f);
GLubyte Crosshair::crosshairRegionColors[3 * 32 * 32] = {0};

GLuint Crosshair::uniformCrosshairTextureLoc;
GLuint Crosshair::uniformCrosshairRegionTextureLoc;
GLint Crosshair::uniformModelLoc;

//attributes UV and position
std::vector<GLushort> Crosshair::vertices = {
    (0 << 10) | (0 << 5) | 0, (0 << 5) | 1,
    (1 << 10) | (0 << 5) | 0, (1 << 5) | 1,
    (0 << 10) | (1 << 5) | 0, (0 << 5) | 0,
    (1 << 10) | (1 << 5) | 0, (1 << 5) | 0,
};
std::vector<GLushort> Crosshair::indexes = {
    0, 1, 2, 2, 1, 3
};

std::unique_ptr<GL::Program> Crosshair::crosshairShader;
std::unique_ptr<GL::VAO> Crosshair::crosshairVAO;

std::unique_ptr<GL::Texture2D> Crosshair::crosshairTexture;
std::unique_ptr<GL::Texture2D> Crosshair::crosshairRegionTexture;

void Crosshair::Initialize(int width, int height) {
    crosshairShader = std::make_unique<GL::Program>("Crosshair");
    crosshairVAO = std::make_unique<GL::VAO>(GL::VAO::Type::VAOcrosshair);

    crosshairTexture = std::make_unique<GL::Texture2D>();
    crosshairRegionTexture = std::make_unique<GL::Texture2D>();


    crosshairShader->bindAttribute(0, "position");
    crosshairShader->bindAttribute(1, "UV");
    crosshairShader->link();

    uniformCrosshairTextureLoc = crosshairShader->getUniformLocation("crosshairTexture");
    uniformCrosshairRegionTextureLoc = crosshairShader->getUniformLocation("crosshairRegionTexture");
    uniformModelLoc = crosshairShader->getUniformLocation("model");

    crosshairTexture->setImage(Image::LoadImage("icons.png"));
    crosshairRegionTexture->set();

    crosshairVAO->Bind();
    crosshairVAO->InitializeVBO(vertices);
    crosshairVAO->InitializeEBO(indexes);
    crosshairVAO->PostInitialization();

    UpdateModel(width, height);
}

void Crosshair::MakeCrosshairRegionTexture() {
    glReadPixels(windowWidth / 2 - crosshairWidth / 2, windowHeight / 2 - crosshairHeight / 2, crosshairWidth, crosshairHeight, GL_RGB, GL_UNSIGNED_BYTE, crosshairRegionColors);
    crosshairRegionTexture->bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, crosshairWidth, crosshairHeight, GL_RGB, GL_UNSIGNED_BYTE, crosshairRegionColors);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Crosshair::UpdateModel(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-static_cast<float>(crosshairWidth) / windowWidth, -static_cast<float>(crosshairHeight) / windowHeight, 0.0f));
    model = glm::scale(model, glm::vec3(static_cast<float>(crosshairWidth) / (windowWidth / 2), static_cast<float>(crosshairHeight) / (windowHeight / 2), 1.0f));
}

void Crosshair::Draw(bool cursorIsMoving, bool cursorIsLocked) {
    if (cursorIsMoving && cursorIsLocked) {
        MakeCrosshairRegionTexture();
    }

    crosshairShader->use();
    crosshairShader->uniformMatrix(uniformModelLoc, model);

    glActiveTexture(GL_TEXTURE0);
    crosshairTexture->bind();
    crosshairShader->uniformTexture(uniformCrosshairTextureLoc, 0);

    glActiveTexture(GL_TEXTURE1);
    crosshairRegionTexture->bind();
    crosshairShader->uniformTexture(uniformCrosshairRegionTextureLoc, 1);

    crosshairVAO->Draw(GL_TRIANGLES);
}
