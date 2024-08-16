#pragma once

#include "../GL/VAO.h"
#include "../GL/Program.h"
#include "../Camera/Camera.h"

#include <vector>

class LineBatch {
    friend class Engine;

public:
    std::vector<glm::vec3> positions_;
    std::vector<glm::vec4> colors_;
    std::vector<unsigned short int> indexes_ = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

    bool draw_box = false;

private:
    std::unique_ptr<GL::Program> shader_;
    std::unique_ptr<GL::VAO> VAO_;

    GLint uniform_projection_loc_;
    GLint uniform_view_loc_;

    int vertex_count_capacity_ = 128;
    int index_count_capacity_ = 128;

private:
    LineBatch();
    ~LineBatch() = default;

public:
    void RewritePositionData();
    void Draw(const Camera& camera) const;
};
