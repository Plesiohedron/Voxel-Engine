#pragma once

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GL/GL.h>

#include <glm/glm.hpp>

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 256
#define CHUNK_DEPTH 16
#define CHUNK_VOLUME (CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH)

namespace GL {
    class VAO {
    private:
        static const unsigned int mVBO_COUNT = 4;

        int mVBO_size;
        int mIBO_size;
        int initializedVBOs_count = 0;

        GLuint mVAO;
        GLuint mVBOs[mVBO_COUNT] = {0};
        GLuint mIBO = 0;
    public:
        enum Type {Chunk, Scope, Line, Test};

        std::vector<float> brithness;
        std::vector<glm::vec2> UVs;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> colors;
        std::vector<unsigned> indexes;

        VAO(Type objectType);
        VAO(const VAO&) = delete;
        ~VAO();

        void bind();
        void draw(unsigned type);

        void initializeVBO_brithness();
        void initializeVBO_UVs();
        void initializeVBO_vertices();
        void initializeVBO_colors();
        void initialize_IBO();

        void postInitialization();

        void deinitializeVBO_brithness();
        void deinitializeVBO_UVs();
        void deinitializeVBO_vertices();
        void deinitializeVBO_colors();
        void deinitialize_IBO();
    };
}