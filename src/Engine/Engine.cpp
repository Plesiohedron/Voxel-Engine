#include "Engine.h"
#include "../Window/Window.h"
#include "../Events/Events.h"
#include "../Camera/Camera.h"
#include "../Chunks/ChunkRenderer.h"

glm::vec3 Engine::posCamera = {0.0f, 0.0f, -5.0f};
float Engine::fovCamera = glm::radians(70.0f);
glm::mat4 Engine::model(1.0f);

GLint Engine::uniformTextureLoc;

GLint Engine::uniformProjectionLoc;
GLint Engine::uniformViewLoc;
GLint Engine::uniformModelLoc;

GL::Texture Engine::textureAtlas;

void Engine::initialize(int width, int height, const char* title) {
    Window::initialize(width, height, title);
    Events::initialize();
    Camera::initialize(posCamera, fovCamera);
    Camera::rotate(0.0f, glm::radians(180.0f), 0.0f);
    Camera::cameraRotationX = glm::radians(180.0f);

    textureAtlas.setImage(Image::LoadImage("Atlas.png"));
}

void Engine::mainLoop() {
    Crosshair crosshair(Window::mWidth, Window::mHeight);

    GL::Program shaderProgram("Shader");
    shaderProgram.bindAttribute(0, "color");
    shaderProgram.bindAttribute(1, "UV");
    shaderProgram.bindAttribute(2, "position");
    shaderProgram.link();

    uniformTextureLoc = shaderProgram.getUniformLocation("texture0");

    uniformProjectionLoc = shaderProgram.getUniformLocation("projection");
    uniformViewLoc = shaderProgram.getUniformLocation("view");
    uniformModelLoc = shaderProgram.getUniformLocation("model");



    Chunk chunk;
    //Chunk* neighbouringСhunks[8] = {nullptr};

    ChunkRenderer::render(chunk);

    GL::VAO chunkVAO(GL::VAO::Type::VAOchunk);

    chunkVAO.bind();
    chunkVAO.initializeVBO(chunk.vertices, chunk.currentVerticesCount);
    chunkVAO.initializeEBO(chunk.indexes, chunk.currentIndexesCount);
    chunkVAO.postInitialization();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glActiveTexture(GL_TEXTURE0);
    glActiveTexture(GL_TEXTURE1);
    glActiveTexture(GL_TEXTURE2);

    float lastTime = static_cast<float>(glfwGetTime());
    float deltaTime = 0.0f;
    float currentTime = 0.0f;

    float speed = 5.0f;

    while (!Window::isShouldClose()) {
        currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (!Window::isIconfied) {

            if (Events::key_isClicked(GLFW_KEY_ESCAPE))
                Window::setShouldClose(true);
            if (Events::key_isClicked(GLFW_KEY_TAB))
                Events::switchCursor();

            if (Events::key_isPressed(GLFW_KEY_W))
                Camera::position += Camera::vectorFront * deltaTime * speed;
            if (Events::key_isPressed(GLFW_KEY_S))
                Camera::position -= Camera::vectorFront * deltaTime * speed;
            if (Events::key_isPressed(GLFW_KEY_D))
                Camera::position += Camera::vectorRight * deltaTime * speed;
            if (Events::key_isPressed(GLFW_KEY_A))
                Camera::position -= Camera::vectorRight * deltaTime * speed;

            if (Window::isResized) {
                crosshair.updateModel(Window::mWidth, Window::mHeight);
                Window::isResized = false;
            }

            if (Events::_cursor_isLocked) {
                Camera::cameraRotationX += -2 * Events::_cursor_delta_x / Window::mHeight;
                Camera::cameraRotationY += -2 * Events::_cursor_delta_y / Window::mHeight;

                if (Camera::cameraRotationY < -glm::radians(90.0f))
                    Camera::cameraRotationY = -glm::radians(90.0f);
                else if (Camera::cameraRotationY > glm::radians(90.0f))
                    Camera::cameraRotationY = glm::radians(90.0f);

                Camera::rotation = glm::mat4(1.0f);
                Camera::rotate(Camera::cameraRotationY, Camera::cameraRotationX, 0.0f);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shaderProgram.use();
            shaderProgram.uniformMatrix(uniformProjectionLoc, Camera::getProjection());
            shaderProgram.uniformMatrix(uniformViewLoc, Camera::getView());
            shaderProgram.uniformMatrix(uniformModelLoc, model);

            glActiveTexture(GL_TEXTURE0);
            textureAtlas.bind();
            shaderProgram.uniformTexture(uniformTextureLoc, 0);

            chunkVAO.draw(GL_TRIANGLES, chunk.currentIndexesCount);

            crosshair.draw(Events::_cursor_isMoving, Events::_cursor_isLocked);
        }

        Window::swapBuffers();
        Events::pollEvents();
    }
}

void Engine::deinitialize() {
    Window::deinitialize();
}
