#include "Engine/Engine.h"

int main() {

    Engine& engine = Engine::Instance(1280, 720, "Window");
    engine.MainLoop();

}