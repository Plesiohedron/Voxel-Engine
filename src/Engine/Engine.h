#pragma once

#include "../Window/Window.h"
#include "../Camera/Camera.h"
#include "../GUI/GUI.h"
#include "../Chunks/ChunkStorage.h"

class Engine {
public:
    inline static Engine& Instance(const int window_width, const int window_height, const char* window_title) {
        static Engine instance{window_width, window_height, window_title};
        return instance;
    }

    void MainLoop();

private:
    Window window_;
    Camera camera_;
    GUI GUI_;
    ChunkStorage* chunk_storage_;

private:
    Engine(const int window_width, const int window_height, const char* window_title);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;
};
