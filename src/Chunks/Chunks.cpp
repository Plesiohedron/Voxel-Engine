#include "Chunks.h"

#include <chrono>

Chunks::Chunks(int radius, const glm::ivec3& center) : storage_({2 * radius - 1, CHUNK_COUNT_IN_HEIGHT, 2 * radius - 1}) {
    storage_.rendering_center = center;
    storage_.rendering_radius = radius;

    models_ = new glm::mat4[storage_.chunk_count];
    VAOs_ = new GL::SChunkVAO*[storage_.sizes.x * storage_.sizes.z];

    if (models_ == nullptr) {
        std::cout << "Bad alloc: models_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (VAOs_ == nullptr) {
        std::cout << "Bad alloc: VAOs_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    for (int i = 0, y = 0; y < storage_.sizes.y; ++y) {
        for (int z = 0; z < storage_.sizes.z; ++z) {
            for (int x = 0; x < storage_.sizes.x; ++x, ++i) {
                models_[i] = glm::translate(glm::mat4(1.0f), glm::vec3(storage_.chunks_[i]->global_coordinates.x * Chunk::WIDTH,
                                                                       storage_.chunks_[i]->global_coordinates.y * Chunk::HEIGHT,
                                                                       storage_.chunks_[i]->global_coordinates.z * Chunk::DEPTH));
            }
        }
    }

    uint32_t* unified_index_data = new uint32_t[UNIFIED_EBO_SIZE];

    if (unified_index_data == nullptr) {
        std::cout << "Bad alloc: unified_index_data" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    for (int i = 0, j = 0; i < UNIFIED_EBO_SIZE; i += Chunk::INDEXES_COUNT_PER_SQUARE, j += Chunk::VERTICES_COUNT_PER_SQUARE) {
        unified_index_data[i] = j + 0;
        unified_index_data[i + 1] = j + 1;
        unified_index_data[i + 2] = j + 2;
        unified_index_data[i + 3] = j + 3;
        unified_index_data[i + 4] = j + 0;
        unified_index_data[i + 5] = j + 2;
    }

    glGenBuffers(1, &unified_EBO_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unified_EBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * UNIFIED_EBO_SIZE, unified_index_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete[] unified_index_data;

    for (int i = 0, z = 0; z < storage_.sizes.z; ++z) {
        for (int x = 0; x < storage_.sizes.x; ++x, ++i) {
            VAOs_[i] = new GL::SChunkVAO{};

            if (VAOs_[i] == nullptr) {
                std::cout << "Bad alloc: VAOs_[i]" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            VAOs_[i]->Bind();
            VAOs_[i]->AllocateVBO(CHUNK_COUNT_IN_HEIGHT * Chunk::STARTING_VOXEL_FACES_CAPACITY * Chunk::VERTICES_COUNT_PER_SQUARE);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unified_EBO_);
        }
    }
    GL::SChunkVAO::Unbind();

    matrices_SSBO_.InitializeMatrices(models_, storage_.chunk_count, 0);


    shader_ = std::make_unique<GL::Program>("Chunks", true);
    shader_->BindAttribute(0, "color");
    shader_->BindAttribute(1, "UV");
    shader_->BindAttribute(2, "position");
    shader_->Link();

    uniform_texture_loc_ = shader_->GetUniformLocation("texture0");
    uniform_projection_loc_ = shader_->GetUniformLocation("projection");
    uniform_view_loc_ = shader_->GetUniformLocation("view");
    uniform_model_index_loc_ = shader_->GetUniformLocation("model_index");

    texture_atlas_ = std::make_unique<GL::Texture3D>();
    texture_atlas_->SetAtlas(Image::LoadImage("Atlas.png"));

    shader_->Use();
    glActiveTexture(GL_TEXTURE0);
    texture_atlas_->Bind();
    shader_->UniformTexture(uniform_texture_loc_, 0);
    GL::Program::Unuse();
}

void Chunks::PollUpdates() {

    for (int i = 0, z = 0; z < storage_.sizes.z; ++z) {
        for (int x = 0; x < storage_.sizes.x; ++x, ++i) {

            int SChunkVAO_capacity = 0;
            bool is_SChunk_modified = false;
            bool is_Chunk_capacity_changed = false;
            for (int y = 0; y < storage_.sizes.y; ++y) {
                // chunks_->chunks_[y][z][x] := chunks_->chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x]
                const Chunk* chunk = storage_.chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x];

                SChunkVAO_capacity += chunk->vertex_data_capacity;

                if (chunk->is_modified) {
                    if (chunk->voxel_faces_capacity * Chunk::VERTICES_COUNT_PER_SQUARE != chunk->vertex_data_capacity) {
                        is_Chunk_capacity_changed = true;
                    }

                    is_SChunk_modified = true;
                }
            }

            if (is_SChunk_modified) {
                VAOs_[i]->Bind();

                if (!is_Chunk_capacity_changed) {
                    int offset = 0;
                    for (int y = 0; y < storage_.sizes.y; ++y) {
                        Chunk* chunk = storage_.chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x];

                        if (chunk->is_modified) {
                            VAOs_[i]->FillVBOSection(chunk->vertex_data, chunk->vertex_data_size, offset);

                            chunk->is_modified = false;

                            chunk->voxel_faces_size = chunk->vertex_data_size / Chunk::VERTICES_COUNT_PER_SQUARE;
                            chunk->vertex_data_size = 0;
                        }

                        offset += chunk->vertex_data_capacity;
                    }
                } else {
                    GLuint temp_VBO = 0;

                    glGenBuffers(1, &temp_VBO);
                    glBindBuffer(GL_ARRAY_BUFFER, temp_VBO);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * SChunkVAO_capacity, nullptr, GL_DYNAMIC_DRAW);
                    glVertexAttribIPointer(0, 4, GL_UNSIGNED_INT, sizeof(Vertex), nullptr);
                    glEnableVertexAttribArray(0);
                    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid*)(4 * sizeof(uint32_t)));
                    glEnableVertexAttribArray(1);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);

                    int unmodified_SChunk_offset = 0;
                    int modified_SChunk_offset = 0;
                    for (int y = 0; y < storage_.sizes.y; ++y) {
                        Chunk* chunk = storage_.chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x];

                        if (chunk->is_modified) {
                            glBindBuffer(GL_ARRAY_BUFFER, temp_VBO);
                            glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertex) * modified_SChunk_offset, sizeof(Vertex) * chunk->vertex_data_size,
                                            chunk->vertex_data);
                            glBindBuffer(GL_ARRAY_BUFFER, 0);

                            chunk->is_modified = false;

                            unmodified_SChunk_offset += chunk->voxel_faces_capacity * Chunk::VERTICES_COUNT_PER_SQUARE;
                            modified_SChunk_offset += chunk->vertex_data_capacity;

                            chunk->voxel_faces_capacity = chunk->vertex_data_capacity / Chunk::VERTICES_COUNT_PER_SQUARE;
                            chunk->voxel_faces_size = chunk->vertex_data_size / Chunk::VERTICES_COUNT_PER_SQUARE;
                            chunk->vertex_data_size = 0;
                        } else {
                            glBindBuffer(GL_COPY_READ_BUFFER, VAOs_[i]->VBO);
                            glBindBuffer(GL_COPY_WRITE_BUFFER, temp_VBO);

                            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, sizeof(Vertex) * unmodified_SChunk_offset,
                                                sizeof(Vertex) * modified_SChunk_offset,
                                                sizeof(Vertex) * chunk->voxel_faces_size * Chunk::VERTICES_COUNT_PER_SQUARE);

                            glBindBuffer(GL_COPY_READ_BUFFER, 0);
                            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

                            unmodified_SChunk_offset += chunk->vertex_data_capacity;
                            modified_SChunk_offset += chunk->vertex_data_capacity;
                        }
                    }

                    glDeleteBuffers(1, &VAOs_[i]->VBO);
                    VAOs_[i]->VBO = temp_VBO;
                }
            }
        }
    }
    GL::SChunkVAO::Unbind();

}

void Chunks::Draw(const Camera& camera) const {
    shader_->Use();
    shader_->UniformMatrix(uniform_projection_loc_, camera.GetProjection());
    shader_->UniformMatrix(uniform_view_loc_, camera.GetView());

    for (int i = 0, z = 0; z < storage_.sizes.z; ++z) {
        for (int x = 0; x < storage_.sizes.x; ++x, ++i) {
            VAOs_[i]->Bind();

            int offset = 0;
            for (int y = 0; y < storage_.sizes.y; ++y) {
                shader_->UniformInt(uniform_model_index_loc_, (y * storage_.sizes.z + z) * storage_.sizes.x + x);
                glDrawElementsBaseVertex(GL_TRIANGLES,
                                         storage_.chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x]->voxel_faces_size *
                                             Chunk::INDEXES_COUNT_PER_SQUARE,
                                         GL_UNSIGNED_INT, nullptr, Chunk::VERTICES_COUNT_PER_SQUARE * offset);
                offset += storage_.chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x]->voxel_faces_capacity;
            }
        }
    }
    GL::SChunkVAO::Unbind();
}

Chunks::~Chunks() {
    glDeleteBuffers(1, &unified_EBO_);
    delete[] models_;

    for (int i = 0; i < storage_.sizes.x * storage_.sizes.z; ++i) {
        delete VAOs_[i];
    }

    delete[] VAOs_;
}
