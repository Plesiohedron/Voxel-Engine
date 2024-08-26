#include "Chunks.h"

#include <chrono>

Chunks::Chunks(int radius, const glm::ivec3& center) : storage_({2 * radius - 1, CHUNK_COUNT_IN_HEIGHT, 2 * radius - 1}) {
    storage_.rendering_center = center;
    storage_.rendering_radius = radius;

    float h_near = tan(glm::radians(90.0f) / 2) * 1;
    float w_near = h_near * Events::window->GetAspect();

    frustum_TL = glm::normalize(glm::vec3(-w_near, h_near, 1));
    frustum_TR = glm::normalize(glm::vec3(w_near, h_near, 1));
    frustum_BR = glm::normalize(glm::vec3(w_near, -h_near, 1));
    frustum_BL = glm::normalize(glm::vec3(-w_near, -h_near, 1));

    frustum_side_edge_length = 512;  // change later

    glm::dmat3 half_volume{frustum_TL, frustum_TR, frustum_BR};

    frustum_volume_sixed = 2 * std::abs(glm::determinant(half_volume));

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


    shader_ = std::make_unique<GL::Program>("Chunks");
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

void Chunks::FrustumCulling(const glm::vec3& camera_position) {
    for (int i = 0; i < storage_.chunk_count; ++i) {
        storage_.chunks_[i]->is_visible = false;
    }

    storage_.FrustumRayCast(camera_position, frustum_TL, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position, frustum_TR, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position, frustum_BR, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position, frustum_BL, frustum_side_edge_length / Chunk::DIRECTION_SIZE);

    for (int global_z = (storage_.rendering_center.z - storage_.sizes.z / 2) * Chunk::DEPTH;
         global_z <= (storage_.rendering_center.z + (storage_.sizes.z + 1) / 2) * Chunk::DEPTH; global_z += Chunk::DEPTH) {
        for (int global_x = (storage_.rendering_center.x - storage_.sizes.x / 2) * Chunk::WIDTH;
             global_x <= (storage_.rendering_center.x + (storage_.sizes.x + 1) / 2) * Chunk::WIDTH; global_x += Chunk::WIDTH) {
            for (int global_y = 0; global_y <= storage_.sizes.y * Chunk::HEIGHT; global_y += Chunk::HEIGHT) {
                glm::dvec3 frustum_top_vertex = static_cast<glm::dvec3>(camera_position);
                glm::dvec3 frustum_TLvertex = static_cast<glm::dvec3>(camera_position + frustum_TL);
                glm::dvec3 frustum_TRvertex = static_cast<glm::dvec3>(camera_position + frustum_TR);
                glm::dvec3 frustum_BRvertex = static_cast<glm::dvec3>(camera_position + frustum_BR);
                glm::dvec3 frustum_BLvertex = static_cast<glm::dvec3>(camera_position + frustum_BL);

                glm::dvec3 separate_vertex = frustum_top_vertex + (glm::dvec3(global_x, global_y, global_z) - frustum_top_vertex) * (1.0 / frustum_side_edge_length);

                glm::dmat3 volume1{separate_vertex - frustum_top_vertex, separate_vertex - frustum_TLvertex, separate_vertex - frustum_TRvertex};
                glm::dmat3 volume2{separate_vertex - frustum_top_vertex, separate_vertex - frustum_BRvertex, separate_vertex - frustum_TRvertex};
                glm::dmat3 volume3{separate_vertex - frustum_top_vertex, separate_vertex - frustum_BRvertex, separate_vertex - frustum_BLvertex};
                glm::dmat3 volume4{separate_vertex - frustum_top_vertex, separate_vertex - frustum_TLvertex, separate_vertex - frustum_BLvertex};
                glm::dmat3 volume5{separate_vertex - frustum_TRvertex,   separate_vertex - frustum_TLvertex, separate_vertex - frustum_BLvertex};
                glm::dmat3 volume6{separate_vertex - frustum_TRvertex,   separate_vertex - frustum_BRvertex, separate_vertex - frustum_BLvertex};

                double result_volume_sixed = std::abs(glm::determinant(volume1)) + std::abs(glm::determinant(volume2)) + std::abs(glm::determinant(volume3)) +
                                             std::abs(glm::determinant(volume4)) + std::abs(glm::determinant(volume5)) + std::abs(glm::determinant(volume6));

                if (frustum_volume_sixed - 0.00005 < result_volume_sixed && result_volume_sixed < frustum_volume_sixed + 0.00005) {
                    int chunk_local_x = global_x / Chunk::WIDTH - storage_.rendering_center.x + storage_.sizes.x / 2;
                    int chunk_local_y = global_y / Chunk::HEIGHT;
                    int chunk_local_z = global_z / Chunk::DEPTH - storage_.rendering_center.z + storage_.sizes.z / 2;

                    if (chunk_local_x - 1 >= 0 && chunk_local_y - 1 >= 0 && chunk_local_z - 1 >= 0) {
                        storage_.chunks_[((chunk_local_y - 1) * storage_.sizes.z + (chunk_local_z - 1)) * storage_.sizes.x + (chunk_local_x - 1)]
                            ->is_visible = true;
                    }
                    if (chunk_local_x - 1 >= 0 && chunk_local_y - 1 >= 0 && chunk_local_z < storage_.sizes.z) {
                        storage_.chunks_[((chunk_local_y - 1) * storage_.sizes.z + chunk_local_z) * storage_.sizes.x + (chunk_local_x - 1)]
                            ->is_visible = true;
                    }
                    if (chunk_local_x - 1 >= 0 && chunk_local_y < storage_.sizes.y && chunk_local_z - 1 >= 0) {
                        storage_.chunks_[(chunk_local_y * storage_.sizes.z + (chunk_local_z - 1)) * storage_.sizes.x + (chunk_local_x - 1)]
                            ->is_visible = true;
                    }
                    if (chunk_local_x - 1 >= 0 && chunk_local_y < storage_.sizes.y && chunk_local_z < storage_.sizes.z) {
                        storage_.chunks_[(chunk_local_y * storage_.sizes.z + chunk_local_z) * storage_.sizes.x + (chunk_local_x - 1)]
                            ->is_visible = true;
                    }
                    if (chunk_local_x < storage_.sizes.x && chunk_local_y - 1 >= 0 && chunk_local_z - 1 >= 0) {
                        storage_.chunks_[((chunk_local_y - 1) * storage_.sizes.z + (chunk_local_z - 1)) * storage_.sizes.x + chunk_local_x]
                            ->is_visible = true;
                    }
                    if (chunk_local_x < storage_.sizes.x && chunk_local_y - 1 >= 0 && chunk_local_z < storage_.sizes.z) {
                        storage_.chunks_[((chunk_local_y - 1) * storage_.sizes.z + chunk_local_z) * storage_.sizes.x + chunk_local_x]
                            ->is_visible = true;
                    }
                    if (chunk_local_x < storage_.sizes.x && chunk_local_y < storage_.sizes.y && chunk_local_z - 1 >= 0) {
                        storage_.chunks_[(chunk_local_y * storage_.sizes.z + (chunk_local_z - 1)) * storage_.sizes.x + chunk_local_x]
                            ->is_visible = true;
                    }
                    if (chunk_local_x < storage_.sizes.x && chunk_local_y < storage_.sizes.y && chunk_local_z < storage_.sizes.z) {
                        storage_.chunks_[(chunk_local_y * storage_.sizes.z + chunk_local_z) * storage_.sizes.x + chunk_local_x]
                            ->is_visible = true;
                    }

                    //std::cout << frustum_volume_sixed << ' ' << result_volume_sixed << ' '
                    //          << chunk_local_x << ' ' << chunk_local_y << ' ' << chunk_local_z << '\n';
                }
            }
        }
    }
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
                if (storage_.chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x]->is_visible) {
                    shader_->UniformInt(uniform_model_index_loc_, (y * storage_.sizes.z + z) * storage_.sizes.x + x);
                    glDrawElementsBaseVertex(GL_TRIANGLES,
                                             storage_.chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x]->voxel_faces_size *
                                                 Chunk::INDEXES_COUNT_PER_SQUARE,
                                             GL_UNSIGNED_INT, nullptr, Chunk::VERTICES_COUNT_PER_SQUARE * offset);
                }
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
