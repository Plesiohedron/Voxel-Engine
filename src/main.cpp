#include <iostream>

#include "Engine/Engine.h"

int main() {

    try {
        Engine::Initialize(1280, 720, "Window");
        Engine::MainLoop();
    } catch (std::runtime_error& error) {
        std::cout << "Fatal Error" << std::endl;
        std::cout << error.what() << std::endl;

        Engine::Deinitialize();
        return -1;
    }

    Engine::Deinitialize();
    return 0;
}