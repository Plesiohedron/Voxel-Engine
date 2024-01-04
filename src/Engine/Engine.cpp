#include "Engine.h"
#include "..\Window\Window.h"
#include "..\Events\Events.h"

void Engine::initialize(int width, int height, const char* title) {
    Window::initialize(width, height, title);
    Events::initialize();
}

void Engine::mainLoop() {
    GL::Program shaderProgram("Shader");
    shaderProgram.BindAttribute(0, "brithness");
    shaderProgram.BindAttribute(1, "UV");
    shaderProgram.BindAttribute(2, "position");
    shaderProgram.BindAttribute(3, "color");
    shaderProgram.Link();

    GL::Texture texture_atlas;
    texture_atlas.SetImage(Image::LoadImage("Atlas.png"));

    GL::VAO sprite(GL::VAO::Test);

    sprite.brithness = {1.0f, 1.0f, 1.0f, 1.0f};
    sprite.UVs = {
        {0.0f, 0.0f},
        {0.0625f, 0.0f},
        {0.0f, 0.0625f},
        {0.0625f, 0.0625f}
    };
    sprite.vertices = {
        {0.5f, 0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
        {-0.5f, 0.5f, 0.0f},
        {-0.5f, -0.5f, 0.0f},
    };
    sprite.indexes = 
    {0, 2, 1,
    1, 2, 3};

    sprite.bind();
    sprite.initializeVBO_brithness();
    sprite.initializeVBO_UVs();
    sprite.initializeVBO_vertices();
    sprite.initialize_IBO();
    sprite.postInitialization();

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);


    while (!Window::isShouldClose()) {
        if (Events::key_isClicked(GLFW_KEY_ESCAPE))
            Window::setShouldClose(true);

        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Use();
        texture_atlas.bind();
        sprite.draw(GL_TRIANGLES);

        Window::swapBuffers();
        Events::pollEvents();
    }
}

void Engine::deinitialize() {
    Window::deinitialize();
}
