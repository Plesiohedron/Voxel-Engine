#include <iostream>

#include "Engine/Engine.h"

int main() {

    try {
        Engine::initialize(1280, 720, "Window");
        Engine::mainLoop();
    } catch (std::runtime_error& error) {
        std::cout << "Fatal Error" << std::endl;
        std::cout << error.what() << std::endl;

        Engine::deinitialize();
        return -1;
    }

    Engine::deinitialize();
    return 0;
}