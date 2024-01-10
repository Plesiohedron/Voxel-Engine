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

        int attributes[3] = {};
        size_t vertex_size = 0;
        unsigned attributes_count = 0;

    public:
        enum Type {VAOchunk, VAOcrosshair, Test};

        VAO(Type type);
        VAO(const VAO&) = delete;
        ~VAO();

        void bind();
        void draw(unsigned primitiveType, unsigned indexes_count);

        void initializeVBO(const GLushort* color, unsigned vertices_count);
        void initializeEBO(const GLushort* data, unsigned indexes_count);

        void postInitialization();

        void deinitializeVBO();
        void deinitializeEBO();
    };
}