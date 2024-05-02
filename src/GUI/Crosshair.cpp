#include "Crosshair.h"

Crosshair::Crosshair() : model_(1.0f) {
    shader_ = std::make_unique<GL::Program>("Crosshair");
    VAO_ = std::make_unique<GL::VAO>();

    texture_ = std::make_unique<GL::Texture2D>();
    region_texture_ = std::make_unique<GL::Texture2D>();


    shader_->BindAttribute(0, "position");
    shader_->BindAttribute(1, "UV");
    shader_->Link();

    uniform_texture_loc_ = shader_->GetUniformLocation("crosshair_texture");
    uniform_region_texture_loc_ = shader_->GetUniformLocation("crosshair_region_texture");
    uniform_model_loc_ = shader_->GetUniformLocation("model");

    texture_->SetImage(Image::LoadImage("icons.png"));
    region_texture_->SetEmpty(WIDTH_, HEIGHT_);

    VAO_->Bind();
    VAO_->InitializeBasicVBO(position_);
    VAO_->InitializeBasicVBO(UV_);
    VAO_->InitializeEBO(indexes_);
    VAO_->PostInitialization();

    UpdateModel();
}

void Crosshair::MakeCrosshairRegionTexture() {
    glReadPixels(Events::window->width / 2 - WIDTH_ / 2, Events::window->height / 2 - HEIGHT_ / 2, WIDTH_, HEIGHT_, GL_RGB, GL_UNSIGNED_BYTE, region_colors_);
    region_texture_->Bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH_, HEIGHT_, GL_RGB, GL_UNSIGNED_BYTE, region_colors_);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Crosshair::UpdateModel() {
    model_ = glm::translate(glm::mat4(1.0f), glm::vec3(-WIDTH_ / Events::window->width, -HEIGHT_ / Events::window->height, 0.0f));
    model_ = glm::scale(model_, glm::vec3(WIDTH_ / (Events::window->width / 2), HEIGHT_ / (Events::window->height / 2), 1.0f));
}

void Crosshair::Draw() {
    if (Events::cursor_is_moving && Events::cursor_is_locked) {
        MakeCrosshairRegionTexture();
    }

    shader_->Use();
    shader_->UniformMatrix(uniform_model_loc_, model_);

    glActiveTexture(GL_TEXTURE0);
    texture_->Bind();
    shader_->UniformTexture(uniform_texture_loc_, 0);

    glActiveTexture(GL_TEXTURE1);
    region_texture_->Bind();
    shader_->UniformTexture(uniform_region_texture_loc_, 1);

    VAO_->Draw(GL_TRIANGLES);
}
