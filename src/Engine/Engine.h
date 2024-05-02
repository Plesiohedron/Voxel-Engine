#pragma once

#include "../GL/Program.h"
#include "../GL/Texture3D.h"
#include "../GL/VAO.h"

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

    GLenum debug_mode_ = GL_TRIANGLES;

    std::unique_ptr<GL::Texture3D> texture_atlas_;
    std::unique_ptr<GL::Program> shader_program_;

    GLint uniform_texture_loc_;
    GLint uniform_projection_loc_;
    GLint uniform_view_loc_;
    GLint uniform_model_loc_;

private:
    Engine(const int window_width, const int window_height, const char* window_title);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;
};
