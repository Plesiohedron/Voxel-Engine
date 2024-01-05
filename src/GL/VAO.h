#pragma once

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GL/GL.h>

#include <glm/glm.hpp>

namespace GL {
    class VAO {
    private:
        GLuint mVAO = 0;
        GLuint mVBO = 0;
        GLuint mEBO = 0;

        int* attributes;
        size_t vertex_size;
        unsigned attributes_count;

    public:
        enum Type {VAOchunk, Test};

        VAO(Type type);
        VAO(const VAO&) = delete;
        ~VAO();

        void bind();
        void draw(unsigned primitiveType, unsigned indexes_count);

        void initializeVBO_vertices(const float* vertices, unsigned vertices_count);
        void initializeEBO(const unsigned* data, unsigned indexes_count);

        void postInitialization();

        void deinitializeVBO_vertices();
        void deinitializeEBO();
    };
}