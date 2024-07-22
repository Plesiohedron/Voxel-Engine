#include "Chunks.h"

glm::ivec3 Chunks::storage_sizes;


Chunks::Chunks(const int radius, const glm::ivec3& center): rendering_radius(radius), rendering_center(center) {
    storage_sizes = {2 * radius - 1, CHUNK_COUNT_IN_HEIGHT, 2 * radius - 1};
    chunk_count = storage_sizes.x * storage_sizes.y * storage_sizes.z;

    models_ = new glm::mat4[chunk_count];
    chunks_ = new Chunk*[chunk_count];
    VAOs_ = new GL::ChunkVAO*[chunk_count];

    for (int i = 0, y = 0; y < storage_sizes.y; ++y) {
        for (int z = 0; z < storage_sizes.z; ++z) {
            for (int x = 0; x < storage_sizes.x; ++x, ++i) {
                VAOs_[i] = new GL::ChunkVAO{};
                chunks_[i] = new Chunk({x, y, z});
                models_[i] = glm::translate(glm::mat4(1.0f), glm::vec3(chunks_[i]->global_coordinates.x * Chunk::WIDTH,
                                                                       chunks_[i]->global_coordinates.y * Chunk::HEIGHT,
                                                                       chunks_[i]->global_coordinates.z * Chunk::DEPTH));
            }
        }
    }

    Chunk::chunk_storage_ = chunks_;
    for (int i = 0; i < chunk_count; ++i) {
        chunks_[i]->CullingChunksJoints();
    }


    shader_ = std::make_unique<GL::Program>("Chunks");
    shader_->BindAttribute(0, "color");
    shader_->BindAttribute(1, "UV");
    shader_->BindAttribute(2, "position");
    shader_->Link();

    uniform_texture_loc_ = shader_->GetUniformLocation("texture0");
    uniform_projection_loc_ = shader_->GetUniformLocation("projection");
    uniform_view_loc_ = shader_->GetUniformLocation("view");
    uniform_model_loc_ = shader_->GetUniformLocation("model");

    texture_atlas_ = std::make_unique<GL::Texture3D>();
    texture_atlas_->SetAtlas(Image::LoadImage("Atlas.png"));
}

void Chunks::Draw(const Camera& camera) const {
    shader_->Use();
    shader_->UniformMatrix(uniform_projection_loc_, camera.GetProjection());
    shader_->UniformMatrix(uniform_view_loc_, camera.GetView());

    glActiveTexture(GL_TEXTURE0);
    texture_atlas_->Bind();
    shader_->UniformTexture(uniform_texture_loc_, 0);

    for (int i = 0; i < chunk_count; ++i) {
        glBindVertexArray(VAOs_[i]->VAO);

        shader_->UniformMatrix(uniform_model_loc_, models_[i]);
        VAOs_[i]->Draw(rendering_mode_);

        glBindVertexArray(0);
    }
}

Chunks::~Chunks() {
    for (int i = 0; i < chunk_count; ++i) {
        delete chunks_[i];
        delete VAOs_[i];
    }

    delete[] VAOs_;
    delete[] chunks_;
    delete[] models_;
}
