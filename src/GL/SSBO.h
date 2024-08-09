#pragma once

#include <vector>

#include <GL/glew.h>
#include <GL/GL.h>
#include <glm/glm.hpp>

namespace GL {
    class SSBO {
    private:
        GLuint SSBO_ = 0;

    public:
        SSBO();
        ~SSBO();

        void InitializeMatrices(const std::vector<glm::mat4>& matrices_data, int binding_point);

    };
} // namespace GL