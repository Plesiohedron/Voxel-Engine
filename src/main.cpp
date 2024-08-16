#include "Engine/Engine.h"

int main() {

    Engine& engine = Engine::Instance(1280, 720, "Window");
    //Engine& engine = Engine::Instance(1920, 1080, "Window");
    engine.MainLoop();

}