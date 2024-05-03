#include "ChunkStorage.h"

glm::ivec3 ChunkStorage::storage_sizes;

std::unordered_map<uint8_t, bool> ChunkStorage::X_planes_;
std::unordered_map<uint8_t, bool> ChunkStorage::Y_planes_;
std::unordered_map<uint8_t, bool> ChunkStorage::Z_planes_;

ChunkStorage::ChunkStorage(const int radius, const glm::ivec3& center)
    : model_(1.0f), rendering_radius(radius), rendering_center(center) {

    storage_sizes = {2 * radius - 1, CHUNKS_COUNT_IN_HEIGHT, 2 * radius - 1};
    chunks_.resize(storage_sizes.x * storage_sizes.y * storage_sizes.z);
    chunks_VAOs_.resize(storage_sizes.x * storage_sizes.y * storage_sizes.z);

    X_planes_.reserve(3 * 2 * 16);
    Y_planes_.reserve(3 * 2 * 16);
    Z_planes_.reserve(3 * 2 * 16);

    for (int y = 0; y < storage_sizes.y; ++y) {
        for (int z = 0; z < storage_sizes.z; ++z) {
            for (int x = 0; x < storage_sizes.z; ++x) {
                chunks_[(y * storage_sizes.z + z) * storage_sizes.x + x] = new Chunk({x, y, z});
                chunks_VAOs_[(y * storage_sizes.z + z) * storage_sizes.x + x] = new GL::VAO();
            }
        }
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

void ChunkStorage::Draw(const Camera& camera) const {
    shader_->Use();
    shader_->UniformMatrix(uniform_projection_loc_, camera.GetProjection());
    shader_->UniformMatrix(uniform_view_loc_, camera.GetView());

    glActiveTexture(GL_TEXTURE0);
    texture_atlas_->Bind();
    shader_->UniformTexture(uniform_texture_loc_, 0);

    for (int y = 0; y < storage_sizes.y; ++y) {
        for (int z = 0; z < storage_sizes.z; ++z) {
            for (int x = 0; x < storage_sizes.z; ++x) {
                const Chunk* chunk = chunks_[(y * storage_sizes.z + z) * storage_sizes.x + x];

                model_ = glm::translate(glm::mat4(1.0f), glm::vec3(chunk->global_coordinate_X * Chunk::WIDTH,
                                                                   chunk->global_coordinate_Y * Chunk::HEIGHT,
                                                                   chunk->global_coordinate_Z * Chunk::DEPTH));

                shader_->UniformMatrix(uniform_model_loc_, model_);

                chunks_VAOs_[(y * storage_sizes.z + z) * storage_sizes.x + x]->Draw(rendering_mode_);
            }
        }
    }
}

ChunkStorage::~ChunkStorage() {
    for (int y = 0; y < storage_sizes.y; ++y) {
        for (int z = 0; z < storage_sizes.z; ++z) {
            for (int x = 0; x < storage_sizes.z; ++x) {
                delete chunks_[(y * storage_sizes.z + z) * storage_sizes.x + x];
                delete chunks_VAOs_[(y * storage_sizes.z + z) * storage_sizes.x + x];
            }
        }
    }
}