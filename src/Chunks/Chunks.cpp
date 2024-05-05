#include "Chunks.h"

glm::ivec3 Chunks::storage_sizes;

std::unordered_map<uint8_t, bool> Chunks::X_planes_;
std::unordered_map<uint8_t, bool> Chunks::Y_planes_;
std::unordered_map<uint8_t, bool> Chunks::Z_planes_;


Chunks::Chunks(const int radius, const glm::ivec3& center): rendering_radius(radius), rendering_center(center) {
    storage_sizes = {2 * radius - 1, CHUNK_COUNT_IN_HEIGHT, 2 * radius - 1};
    chunk_count = storage_sizes.x * storage_sizes.y * storage_sizes.z;

    vertex_data_capacity_ = chunk_count * Chunk::MAXIMUM_VOXEL_FACES_COUNT * Chunk::VERTEX_ATTRIBUTES_COUNT * Chunk::VERTICES_COUNT_PER_SQUARE;
    index_data_capacity_ = chunk_count * Chunk::MAXIMUM_VOXEL_FACES_COUNT * Chunk::INDEXES_COUNT_PER_SQUARE;
    indirect_command_data_capacity_ = chunk_count;

    models_ = new glm::mat4[chunk_count];
    chunks_ = new Chunk*[chunk_count];

    unified_voxel_vertex_data_ = new GLushort[vertex_data_capacity_];
    unified_voxel_index_data_ = new GLushort[index_data_capacity_];
    unified_voxel_indirect_command_data_ = new GL::IndirectCommand[indirect_command_data_capacity_];

    X_planes_.reserve(3 * 2 * Chunk::WIDTH);
    Y_planes_.reserve(3 * 2 * Chunk::HEIGHT);
    Z_planes_.reserve(3 * 2 * Chunk::DEPTH);

    for (int i = 0, y = 0; y < storage_sizes.y; ++y) {
        for (int z = 0; z < storage_sizes.z; ++z) {
            for (int x = 0; x < storage_sizes.x; ++x, ++i) {
                chunks_[i] = new Chunk({x, y, z});
                models_[i] = glm::translate(glm::mat4(1.0f), glm::vec3(chunks_[i]->global_coordinate_X * Chunk::WIDTH,
                                                                       chunks_[i]->global_coordinate_Y * Chunk::HEIGHT,
                                                                       chunks_[i]->global_coordinate_Z * Chunk::DEPTH));
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

    unified_VAO_ = std::make_unique<GL::SpecialVAO>();

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

    glBindVertexArray(unified_VAO_->VAO);

    for (int i = 0; i < Chunk::VERTEX_ATTRIBUTES_COUNT; ++i) {
        glEnableVertexAttribArray(i);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unified_VAO_->EBO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, unified_VAO_->IB);

    for (int i = 0; i < chunk_count; ++i) {
        shader_->UniformMatrix(uniform_model_loc_, models_[i]);
        glMultiDrawElementsIndirect(rendering_mode_, GL_UNSIGNED_SHORT, reinterpret_cast<const void*>(i * sizeof(GL::IndirectCommand)), 1, 0);
    }

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    for (int i = 0; i < Chunk::VERTEX_ATTRIBUTES_COUNT; ++i) {
        glDisableVertexAttribArray(i);
    }

    glBindVertexArray(0);
}

Chunks::~Chunks() {
    for (int i = 0; i < chunk_count; ++i) {
        delete chunks_[i];
    }
    delete[] chunks_;
    delete[] models_;

    delete[] unified_voxel_vertex_data_;
    delete[] unified_voxel_index_data_;
    delete[] unified_voxel_indirect_command_data_;
}
