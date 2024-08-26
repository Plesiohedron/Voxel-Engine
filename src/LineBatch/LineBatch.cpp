#include "LineBatch.h"

LineBatch::LineBatch() {
    positions_.reserve(vertex_count_capacity_);
    colors_.reserve(vertex_count_capacity_);
    indexes_.reserve(index_count_capacity_);

    for (int i = 0; i < 8; ++i) {
        positions_.push_back({0.f, 0.f, 0.f});
        colors_.push_back({0.f, 0.f, 0.f, 0.5f});
    }

    shader_ = std::make_unique<GL::Program>("Lines");
    VAO_ = std::make_unique<GL::VAO>();

    shader_->BindAttribute(0, "position");
    shader_->BindAttribute(1, "color");
    shader_->Link();

    uniform_projection_loc_ = shader_->GetUniformLocation("projection");
    uniform_view_loc_ = shader_->GetUniformLocation("view");

    VAO_->Bind();
    VAO_->AllocateFloatVBO(positions_);
    VAO_->AllocateFloatVBO(colors_);
    VAO_->AllocateEBO(indexes_);

    VAO_->AssignFloatVBO(positions_, 0);
    VAO_->AssignFloatVBO(colors_, 1);
    VAO_->AssignEBO(indexes_);
    GL::VAO::Unbind();

}

void LineBatch::RewritePositionData() {
    VAO_->Bind();
    VAO_->AssignFloatVBO(positions_, 0);
    GL::VAO::Unbind();
}

void LineBatch::Draw(const Camera& camera) const {
    shader_->Use();
    shader_->UniformMatrix(uniform_projection_loc_, camera.GetProjection());
    shader_->UniformMatrix(uniform_view_loc_, camera.GetView());

    VAO_->Draw(GL_LINES);
}