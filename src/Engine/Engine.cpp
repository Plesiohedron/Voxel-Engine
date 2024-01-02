#include "Engine.h"
#include "..\Window\Window.h"
#include "..\Events\Events.h"

void Engine::initialize(int width, int height, const char* title) {
    Window::initialize(width, height, title);
    Events::initialize();
}

void Engine::mainLoop() {
    while (!Window::isShouldClose()) {
        Events::pollEvents();
        if (Events::key_isClicked(GLFW_KEY_ESCAPE))
            Window::setShouldClose(true);

        glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Window::swapBuffers();
    }
}

void Engine::deinitialize() {
    Window::deinitialize();
}
