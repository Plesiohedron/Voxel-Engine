#include "Engine.h"
#include "../Window/Window.h"
#include "../Events/Events.h"
#include "../Camera/Camera.h"
#include "../Chunks/ChunkRenderer.h"

std::vector<GLushort> Engine::vertices;
std::vector<GLushort> Engine::indexes;

glm::vec3 Engine::posCamera = {0.0f, 0.0f, -5.0f};
float Engine::fovCamera = glm::radians(70.0f);
glm::mat4 Engine::model(1.0f);

GLint Engine::uniformTextureLoc;
GLint Engine::uniformProjectionLoc;
GLint Engine::uniformViewLoc;
GLint Engine::uniformModelLoc;

std::unique_ptr<GL::Texture3D> Engine::textureAtlas;
std::unique_ptr<GL::Program> Engine::shaderProgram;

void Engine::Initialize(int width, int height, const char* title) {
    Window::Initialize(width, height, title);
    Events::Initialize();
    Camera::initialize(posCamera, fovCamera);
    Camera::rotate(0.0f, glm::radians(180.0f), 0.0f);
    Camera::cameraRotationX = glm::radians(180.0f);
    Crosshair::Initialize(Window::mWidth, Window::mHeight);

    shaderProgram = std::make_unique<GL::Program>("Shader");
    shaderProgram->bindAttribute(0, "color");
    shaderProgram->bindAttribute(1, "UV");
    shaderProgram->bindAttribute(2, "position");
    shaderProgram->link();

    uniformTextureLoc = shaderProgram->getUniformLocation("texture0");
    uniformProjectionLoc = shaderProgram->getUniformLocation("projection");
    uniformViewLoc = shaderProgram->getUniformLocation("view");
    uniformModelLoc = shaderProgram->getUniformLocation("model");

    textureAtlas = std::make_unique<GL::Texture3D>();
    textureAtlas->setImage(Image::LoadImage("Atlas.png"));

    vertices.reserve(6 * ATTRIBUTES_COUNT * (CHUNK_H + 1) * (CHUNK_D + 1) * (CHUNK_W + 1));
    indexes.reserve(6 * (CHUNK_H + 1) * (CHUNK_D + 1) * (CHUNK_W + 1));
}

void Engine::MainLoop() {
    Chunk chunk;
    Chunk* neighbouringСhunks[8] = {nullptr};

    ChunkRenderer::Render(chunk, vertices, indexes);

    GL::VAO chunkVAO(GL::VAO::Type::VAOchunk);

    chunkVAO.Bind();
    chunkVAO.InitializeVBO(vertices);
    chunkVAO.InitializeEBO(indexes);
    chunkVAO.PostInitialization();

    /*
    GL::VAO test(GL::VAO::Type::Test);

    GLushort vertices[12] = {
        ((0 << 10) | (1 << 5) | 0), ((1 << 10) | (0 << 5) | 0), ((15 << 12) | (15 << 8) | (15 << 4) | 15),
        ((0 << 10) | (0 << 5) | 0), ((1 << 10) | (0 << 5) | 1), ((15 << 12) | (15 << 8) | (15 << 4) | 15),

        ((2 << 10) | (1 << 5) | 0), ((1 << 10) | (2 << 5) | 0), ((15 << 12) | (15 << 8) | (15 << 4) | 15),
        ((2 << 10) | (0 << 5) | 0), ((1 << 10) | (2 << 5) | 1), ((15 << 12) | (15 << 8) | (15 << 4) | 15),
    };

    GLushort indexes[6] = {
        0, 2, 1, 1, 2, 3
    };

    test.Bind();
    test.InitializeVBO(vertices, 12);
    test.InitializeEBO(indexes, 6);
    test.PostInitialization();
    */

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);


    float lastTime = static_cast<float>(glfwGetTime());
    float deltaTime = 0.0f;
    float currentTime = 0.0f;

    float speed = 5.0f;

    while (!Window::IsShouldClose()) {
        currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (!Window::isIconfied) {
            if (Events::KeyIsClicked(GLFW_KEY_ESCAPE)) {
                Window::SetShouldClose(true);
            }
            if (Events::KeyIsClicked(GLFW_KEY_TAB)) {
                Events::SwitchCursor();
            }

            if (Events::KeyIsPressed(GLFW_KEY_W)) {
                Camera::position += Camera::vectorFront * deltaTime * speed;
            }
            if (Events::KeyIsPressed(GLFW_KEY_S)) {
                Camera::position -= Camera::vectorFront * deltaTime * speed;
            }
            if (Events::KeyIsPressed(GLFW_KEY_D)) {
                Camera::position += Camera::vectorRight * deltaTime * speed;
            }
            if (Events::KeyIsPressed(GLFW_KEY_A)) {
                Camera::position -= Camera::vectorRight * deltaTime * speed;
            }

            if (Window::isResized) {
                Crosshair::UpdateModel(Window::mWidth, Window::mHeight);
                Window::isResized = false;
            }

            if (Events::cursor_is_locked) {
                Camera::cameraRotationX += -2 * Events::cursor_delta_x / Window::mHeight;
                Camera::cameraRotationY += -2 * Events::cursor_delta_y / Window::mHeight;

                if (Camera::cameraRotationY < -glm::radians(90.0f)) {
                    Camera::cameraRotationY = -glm::radians(90.0f);
                }
                else if (Camera::cameraRotationY > glm::radians(90.0f)) {
                    Camera::cameraRotationY = glm::radians(90.0f);
                }

                Camera::rotation = glm::mat4(1.0f);
                Camera::rotate(Camera::cameraRotationY, Camera::cameraRotationX, 0.0f);
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shaderProgram->use();
            shaderProgram->uniformMatrix(uniformProjectionLoc, Camera::getProjection());
            shaderProgram->uniformMatrix(uniformViewLoc, Camera::getView());
            shaderProgram->uniformMatrix(uniformModelLoc, model);

            glActiveTexture(GL_TEXTURE0);
            textureAtlas->bind();
            shaderProgram->uniformTexture(uniformTextureLoc, 0);

            if (Events::KeyIsPressed(GLFW_KEY_F)) {
                chunkVAO.Draw(GL_LINES);
            } else {
                chunkVAO.Draw(GL_TRIANGLES);
            }

            Crosshair::Draw(Events::cursor_is_moving, Events::cursor_is_locked);
        }

        Window::SwapBuffers();
        Events::PollEvents();
    }
}

void Engine::Deinitialize() {
    Window::Deinitialize();
}
