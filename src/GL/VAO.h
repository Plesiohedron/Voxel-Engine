#pragma once

#include <GL/glew.h>
#include <GL/GL.h>

#include <vector>
#include <glm/glm.hpp>

namespace GL {
    class VAO {
    private:
        std::vector<GLuint> VBOs_;
        GLuint VAO_ = 0;
        GLuint EBO_ = 0;

        unsigned int indexes_count_ = 0;
        unsigned int attributes_count_ = 0;

    public:
        VAO();
        VAO(const VAO&) = delete;
        ~VAO();

        void Bind() const;
        static void Unbind();
        void Draw(GLenum primitive_type) const;

        template <typename T>
        void InitializeFloatVBO(const std::vector<T>& vertex_data) {
            GLuint VBO;

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(T), vertex_data.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(VBOs_.size(), sizeof(T) / sizeof(float), GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(VBOs_.size());

            VBOs_.push_back(VBO);
        };

        template <typename U>
        void AllocateFloatVBO(const std::vector<U>& vertex_data) {
            GLuint VBO;

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertex_data.capacity() * sizeof(U), nullptr, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(VBOs_.size(), sizeof(U) / sizeof(float), GL_FLOAT, GL_FALSE, 0, nullptr);
            glEnableVertexAttribArray(VBOs_.size());

            VBOs_.push_back(VBO);
        };

        template <typename V>
        void AssignFloatVBO(const std::vector<V>& vertex_data, GLuint attribute) {
            glBindBuffer(GL_ARRAY_BUFFER, VBOs_[attribute]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(V) * vertex_data.size(), vertex_data.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        };

        void InitializeEBO(const std::vector<unsigned short int>& index_data);
        void AllocateEBO(const std::vector<unsigned short int>& index_data);
        void AssignEBO(const std::vector<unsigned short int>& index_data);

        void DeinitializeVBO();
        void DeinitializeEBO();
    };
} // namespace GL
