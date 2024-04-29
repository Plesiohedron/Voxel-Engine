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
        size_t mIndexesCount = 0;

    public:
        enum Type {VAOchunk, VAOcrosshair, Test};

        VAO(Type type);
        VAO(const VAO&) = delete;
        ~VAO();

        void Bind();
        void Draw(unsigned primitiveType);

        void InitializeVBO(const std::vector<GLushort>& verticesData);
        void InitializeEBO(const std::vector<GLushort>& indexesData);

        void PostInitialization();

        void DeinitializeVBO();
        void DeinitializeEBO();
    };
}