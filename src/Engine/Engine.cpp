#include "Engine.h"
#include "../Window/Window.h"
#include "../Events/Events.h"
#include "../Camera/Camera.h"
#include "../Chunks/ChunkRenderer.h"

glm::vec3 Engine::posCamera = {0.0f, 0.0f, -5.0f};
float Engine::fovCamera = glm::radians(70.0f);
glm::mat4 Engine::model(1.0f);

GL::Texture textureAtlas;

GLint Engine::uniformProjection;
GLint Engine::uniformView;
GLint Engine::uniformModel;

void Engine::initialize(int width, int height, const char* title) {
    Window::initialize(width, height, title);
    Events::initialize();
    Camera::initialize(posCamera, fovCamera);
    Camera::rotate(0.0f, glm::radians(180.0f), 0.0f);
    Camera::cameraRotationX = glm::radians(180.0f);

    textureAtlas.setImage(Image::LoadImage("Atlas.png"));
}

void Engine::mainLoop() {
    GL::Program shaderProgram("Shader");
    shaderProgram.bindAttribute(0, "color");
    shaderProgram.bindAttribute(1, "UV");
    shaderProgram.bindAttribute(2, "position");
    shaderProgram.link();

    uniformProjection = shaderProgram.getUniformLocation("projection");
    uniformView = shaderProgram.getUniformLocation("view");
    uniformModel = shaderProgram.getUniformLocation("model");

    Chunk chunk;
    //Chunk* neighbouringСhunks[8] = {nullptr};

    ChunkRenderer::render(chunk);

    GL::VAO chunkVAO(GL::VAO::Type::Test);

    chunkVAO.bind();
    chunkVAO.test(chunk.vertices, chunk.currentVerticesCount);
    chunkVAO.initializeEBO(chunk.indexes, chunk.currentIndexesCount);
    chunkVAO.postInitialization();

    /*chunkVAO.bind();
    chunkVAO.initializeVBO_vertices(chunk.vertices, chunk.currentVerticesCount);
    chunkVAO.initializeEBO(chunk.indexes, chunk.currentIndexesCount);
    chunkVAO.postInitialization();*/

    /*GL::VAO test(GL::VAO::Type::Test);

    unsigned vertices_count = 6 * 12;
    unsigned indexes_count = 6 * 6;

    //        r             g             b        a      UVx      UVy     x            y        z
    GLushort* vertices = new GLushort[vertices_count]{
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 0, (1 << 10) | (1 << 5) | 0,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 1, (1 << 10) | (0 << 5) | 0,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 0, (0 << 10) | (1 << 5) | 0,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 1, (0 << 10) | (0 << 5) | 0,

        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 0, (1 << 10) | (1 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 0, (0 << 10) | (1 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 1, (1 << 10) | (0 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 1, (0 << 10) | (0 << 5) | 1,

        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 1, (1 << 10) | (1 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 0, (1 << 10) | (1 << 5) | 0,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 1, (0 << 10) | (1 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 0, (0 << 10) | (1 << 5) | 0,

        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 0, (1 << 10) | (0 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 0, (0 << 10) | (0 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 1, (1 << 10) | (0 << 5) | 0,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 1, (0 << 10) | (0 << 5) | 0,

        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 0, (1 << 10) | (1 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 1, (1 << 10) | (0 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 0, (1 << 10) | (1 << 5) | 0,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 1, (1 << 10) | (0 << 5) | 0,

        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 0, (0 << 10) | (1 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 0, (0 << 10) | (1 << 5) | 0,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (1 << 5) | 1, (0 << 10) | (0 << 5) | 1,
        (15 << 12) | (15 << 8) | (15 << 4) | 15, (0 << 5) | 1, (0 << 10) | (0 << 5) | 0,
    };

    GLushort* indexes = new GLushort[indexes_count]{
        0, 1, 2, 2, 1, 3,   4, 5, 6, 6, 5, 7,   8, 9, 10, 10, 9, 11,   12, 13, 14, 14, 13, 15,   16, 17, 18, 18, 17, 19,   20, 21, 22, 22, 21, 23
    };

    test.bind();
    test.test(vertices, vertices_count);
    test.initializeEBO(indexes, indexes_count);
    test.postInitialization();*/

    //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));

    glEnable(GL_DEPTH_TEST);
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

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shaderProgram.use();
            shaderProgram.uniformMatrix(uniformProjection, Camera::getProjection());
            shaderProgram.uniformMatrix(uniformView, Camera::getView());
            shaderProgram.uniformMatrix(uniformModel, model);

            textureAtlas.bind();

            //test.draw(GL_TRIANGLES, indexes_count);
            chunkVAO.draw(GL_TRIANGLES, chunk.currentIndexesCount);
        }

        Window::swapBuffers();
        Events::pollEvents();
    }

    //delete[] vertices;
    //delete[] indexes;
}

void Engine::deinitialize() {
    Window::deinitialize();
}
