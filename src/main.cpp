#include <iostream>

#include "Engine/Engine.h"

int main() {

    try {
        Engine& engine = Engine::Instance(1280, 720, "Window");
        engine.MainLoop();
    } catch (const GLFWError& e) {
        std::cerr << e.what() << std::endl;
    } catch (const OpenGLError& e) {
        std::cerr << e.what() << std::endl;
    } catch (const STBImageError& e) {
        std::cerr << e.what() << std::endl;
    }

}