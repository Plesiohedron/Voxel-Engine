#include "Engine.h"
#include "../Window/Window.h"
#include "../Events/Events.h"
#include "../Camera/Camera.h"
#include "../Chunks/ChunkRenderer.h"

glm::vec3 Engine::posCamera = {0.0f, 0.0f, 0.0f};
float Engine::fovCamera = glm::radians(70.0f);
glm::mat4 Engine::model(1.0f);

void Engine::initialize(int width, int height, const char* title) {
    Window::initialize(width, height, title);
    Events::initialize();
    Camera::initialize(posCamera, fovCamera);
}

void Engine::mainLoop() {
    GL::Program shaderProgram("Shader");
    shaderProgram.bindAttribute(0, "brithness");
    shaderProgram.bindAttribute(1, "UV");
    shaderProgram.bindAttribute(2, "position");
    shaderProgram.link();

    GL::Texture textureAtlas;
    textureAtlas.setImage(Image::LoadImage("Atlas.png"));
    textureAtlas.bind();

    /*Chunk chunk;
    Chunk* neighbouringСhunks[8] = {nullptr};

    ChunkRenderer::render(chunk, (const Chunk**) neighbouringСhunks);

    GL::VAO chunkVAO(GL::VAO::Type::VAOchunk);

    chunkVAO.bind();
    chunkVAO.initializeVBO_vertices(chunk.vertices, chunk.currentVerticesCount);
    chunkVAO.initializeEBO(chunk.indexes, chunk.currentIndexesCount);
    chunkVAO.postInitialization();*/

    GL::VAO test(GL::VAO::Type::Test);

    float* sprite = new float[6 * 4]{1.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f,
                           1.0f, 0.0625f, 0.0f, 0.5f, -0.5f, 0.0f,
                           1.0f, 0.0f, 0.0625f, -0.5f, 0.5f, 0.0f,
                           1.0f, 0.0625f, 0.0625f, -0.5f, -0.5f, 0.0f};

    unsigned* indexes = new unsigned[6]{0, 2, 1,
                                        1, 2, 3};

    test.bind();
    test.initializeVBO_vertices(sprite, 4);
    test.initializeEBO(indexes, 6);
    test.postInitialization();

    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);

    float lastTime = static_cast<float>(glfwGetTime());
    float deltaTime = 0.0f;
    float currentTime = 0.0f;

    float speed = 5.0f;

    while (!Window::isShouldClose()) {
        currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (!Window::mIconfied) {

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

            glClear(GL_COLOR_BUFFER_BIT);

            shaderProgram.use();
            shaderProgram.uniformMatrix("projection", Camera::getProjection());
            shaderProgram.uniformMatrix("view", Camera::getView());
            shaderProgram.uniformMatrix("model", model);
            test.draw(GL_TRIANGLES, 6);
            //chunkVAO.draw(GL_TRIANGLES, chunk.currentIndexesCount);
        }

        Window::swapBuffers();
        Events::pollEvents();
    }

    delete[] sprite;
    delete[] indexes;
}

void Engine::deinitialize() {
    Window::deinitialize();
}
