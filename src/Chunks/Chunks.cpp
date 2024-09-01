#include "Chunks.h"

#include <chrono>

Chunks::Chunks(int radius, const glm::ivec3& center) : storage_({2 * radius - 1, CHUNK_COUNT_IN_HEIGHT, 2 * radius - 1}) {
    storage_.rendering_center = center;
    storage_.rendering_radius = radius;

    frustum_TL = glm::normalize(glm::vec3(-w_near, h_near, 1));
    frustum_TR = glm::normalize(glm::vec3(w_near, h_near, 1));
    frustum_BR = glm::normalize(glm::vec3(w_near, -h_near, 1));
    frustum_BL = glm::normalize(glm::vec3(-w_near, -h_near, 1));

    frustum_side_edge_length = 512;  // change later

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

    #ifdef FRUSTUM_CULLING_GPU
    program_ = std::make_unique<CL::Program>("FrustumCulling");

    program_->CreateBuffer(CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float) * 18);
    program_->CreateBuffer(CL_MEM_WRITE_ONLY, sizeof(char) * storage_.chunk_count);
    input_buffer.resize(18);

    program_->CreateKernel("Culling", {static_cast<size_t>(storage_.sizes.x + 1), static_cast<size_t>(storage_.sizes.z + 1), static_cast<size_t>(storage_.sizes.y + 1)}, {0, 1});
    #endif
}

void Chunks::FrustumCulling(const glm::vec3& camera_position) const {
    #ifdef FRUSTUM_CULLING_GPU
    input_buffer[0] = rotation[0][0];
    input_buffer[1] = rotation[1][0];
    input_buffer[2] = rotation[2][0];
    input_buffer[3] = rotation[0][1];
    input_buffer[4] = rotation[1][1];
    input_buffer[5] = rotation[2][1];
    input_buffer[6] = rotation[0][2];
    input_buffer[7] = rotation[1][2];
    input_buffer[8] = rotation[2][2];
    input_buffer[9] = camera_position.x / Chunk::DIRECTION_SIZE;
    input_buffer[10] = camera_position.y / Chunk::DIRECTION_SIZE;
    input_buffer[11] = camera_position.z / Chunk::DIRECTION_SIZE;
    input_buffer[12] = static_cast<float>(storage_.rendering_center.x);
    input_buffer[13] = static_cast<float>(storage_.rendering_center.z);
    input_buffer[14] = static_cast<float>(storage_.sizes.x);
    input_buffer[15] = static_cast<float>(storage_.sizes.z);
    input_buffer[16] = Events::window->GetAspect() / tan(glm::radians(90.0f) / 2);
    input_buffer[17] = frustum_side_edge_length / Chunk::DIRECTION_SIZE;
    output_buffer = std::vector<char>(storage_.chunk_count, 0);

    program_->WriteToBuffer(input_buffer, 0);
    program_->WriteToBuffer(output_buffer, 1);
    program_->EnqueueKernel(0);

    for (int i = 0; i < storage_.chunk_count; ++i) {
        storage_.chunks_[i]->is_visible = false;
    }

    storage_.FrustumRayCast(camera_position / Chunk::DIRECTION_SIZE, frustum_TL, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position / Chunk::DIRECTION_SIZE, frustum_TR, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position / Chunk::DIRECTION_SIZE, frustum_BR, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position / Chunk::DIRECTION_SIZE, frustum_BL, frustum_side_edge_length / Chunk::DIRECTION_SIZE);

    program_->WaitQueue();
    program_->ReadFromBuffer(output_buffer, 1);

    for (int i = 0; i < storage_.chunk_count; ++i) {
        if (output_buffer[i]) {
            storage_.chunks_[i]->is_visible = true;
        }
    }
    #else
    for (int i = 0; i < storage_.chunk_count; ++i) {
        storage_.chunks_[i]->is_visible = false;
    }

    storage_.FrustumRayCast(camera_position / Chunk::DIRECTION_SIZE, frustum_TL, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position / Chunk::DIRECTION_SIZE, frustum_TR, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position / Chunk::DIRECTION_SIZE, frustum_BR, frustum_side_edge_length / Chunk::DIRECTION_SIZE);
    storage_.FrustumRayCast(camera_position / Chunk::DIRECTION_SIZE, frustum_BL, frustum_side_edge_length / Chunk::DIRECTION_SIZE);

    int chunk_local_x;
    int chunk_local_y;
    int chunk_local_z;

    for (int global_z = (storage_.rendering_center.z - storage_.sizes.z / 2);
         global_z <= (storage_.rendering_center.z + (storage_.sizes.z + 1) / 2); ++global_z) {
        for (int global_x = (storage_.rendering_center.x - storage_.sizes.x / 2);
             global_x <= (storage_.rendering_center.x + (storage_.sizes.x + 1) / 2); ++global_x) {
            for (int global_y = 0; global_y <= storage_.sizes.y; ++global_y) {
                glm::vec3 vertex = glm::vec3(rotation * glm::vec4(glm::vec3(global_x, global_y, global_z) - camera_position / Chunk::DIRECTION_SIZE, 1));

                if (vertex.z >= -frustum_side_edge_length / Chunk::DIRECTION_SIZE &&
                    Events::window->GetAspect() / tan(glm::radians(90.0f) / 2) * vertex.z <= -std::max(std::abs(vertex.x), std::abs(vertex.y))) {

                    chunk_local_x = global_x - storage_.rendering_center.x + storage_.sizes.x / 2;
                    chunk_local_y = global_y;
                    chunk_local_z = global_z - storage_.rendering_center.z + storage_.sizes.z / 2;

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
                }
            }
        }
    }
    #endif
}

void Chunks::ACCA(const glm::vec3& camera_position) {
    for (int i = 0; i < storage_.chunk_count; ++i) {
        storage_.chunks_[i]->is_reachable = 0;
    }

    Chunk* current_chunk = storage_.GetChunkByVoxel(floor(camera_position.x), floor(camera_position.y), floor(camera_position.z));
    if (current_chunk == nullptr) {
        return;
    }

    current_chunk->is_reachable = 0b111111;

    glm::ivec3 starting_chunk_coordinates = current_chunk->global_coordinates;
    glm::ivec3 local_chunk_coordinates = {current_chunk->global_coordinates.x - storage_.rendering_center.x + storage_.sizes.x / 2,
                                          current_chunk->global_coordinates.y,
                                          current_chunk->global_coordinates.z - storage_.rendering_center.z + storage_.sizes.z / 2};
    int reached_from;

    Chunk* next_chunk;

    if (((current_chunk->reachability_code >> 0) & 1)) {
        if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
            if (next_chunk->is_visible) {
                ACCA_queue_.push({local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 1});
                next_chunk->is_reachable = 0b000010;
            }
        }
    }
    if (((current_chunk->reachability_code >> 7) & 1)) {
        if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
            if (next_chunk->is_visible) {
                ACCA_queue_.push({local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 0});
                next_chunk->is_reachable = 0b000001;
            }
        }
    }
    if (((current_chunk->reachability_code >> 14) & 1)) {
        if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z)) {
            if (next_chunk->is_visible) {
                ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z, 3});
                next_chunk->is_reachable = 0b001000;
            }
        }
    }
    if (((current_chunk->reachability_code >> 21) & 1)) {
        if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z)) {
            if (next_chunk->is_visible) {
                ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z, 2});
                next_chunk->is_reachable = 0b000100;
            }
        }
    }
    if (((current_chunk->reachability_code >> 28) & 1)) {
        if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1)) {
            if (next_chunk->is_visible) {
                ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1, 5});
                next_chunk->is_reachable = 0b100000;
            }
        }
    }
    if (((current_chunk->reachability_code >> 35) & 1)) {
        if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1)) {
            if (next_chunk->is_visible) {
                ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1, 4});
                next_chunk->is_reachable = 0b010000;
            }
        }
    }

    while (!ACCA_queue_.empty()) {
        local_chunk_coordinates = {ACCA_queue_.front().x, ACCA_queue_.front().y, ACCA_queue_.front().z};
        reached_from = ACCA_queue_.front().w;
        ACCA_queue_.pop();

        current_chunk = storage_.chunks_[(local_chunk_coordinates.y * storage_.sizes.z + local_chunk_coordinates.z) * storage_.sizes.x + local_chunk_coordinates.x];

        if (reached_from == 0) {
            if (((current_chunk->reachability_code >> 1) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000001))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 0});
                            next_chunk->is_reachable |= 0b000001;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 2) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b001000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z, 3});
                            next_chunk->is_reachable |= 0b001000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 3) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000100))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z, 2});
                            next_chunk->is_reachable |= 0b000100;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 4) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b100000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1, 5});
                            next_chunk->is_reachable |= 0b100000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 5) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b010000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1, 4});
                            next_chunk->is_reachable |= 0b010000;
                        }
                    }
                }
            }
        } else if (reached_from == 1) {
            if (((current_chunk->reachability_code >> 6) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000010))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 1});
                            next_chunk->is_reachable |= 0b000010;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 8) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b001000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z, 3});
                            next_chunk->is_reachable |= 0b001000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 9) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000100))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z, 2});
                            next_chunk->is_reachable |= 0b000100;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 10) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b100000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1, 5});
                            next_chunk->is_reachable |= 0b100000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 11) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b010000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1, 4});
                            next_chunk->is_reachable |= 0b010000;
                        }
                    }
                }
            }
        } else if (reached_from == 2) {
            if (((current_chunk->reachability_code >> 12) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000010))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 1});
                            next_chunk->is_reachable |= 0b000010;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 13) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000001))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 0});
                            next_chunk->is_reachable |= 0b000001;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 15) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000100))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z, 2});
                            next_chunk->is_reachable |= 0b000100;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 16) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b100000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1, 5});
                            next_chunk->is_reachable |= 0b100000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 17) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b010000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1, 4});
                            next_chunk->is_reachable |= 0b010000;
                        }
                    }
                }
            }
        } else if (reached_from == 3) {
            if (((current_chunk->reachability_code >> 18) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000010))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 1});
                            next_chunk->is_reachable |= 0b000010;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 19) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000001))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 0});
                            next_chunk->is_reachable |= 0b000001;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 20) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b001000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z, 3});
                            next_chunk->is_reachable |= 0b001000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 22) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b100000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1, 5});
                            next_chunk->is_reachable |= 0b100000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 23) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b010000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1, 4});
                            next_chunk->is_reachable |= 0b010000;
                        }
                    }
                }
            }
        } else if (reached_from == 4) {
            if (((current_chunk->reachability_code >> 24) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000010))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 1});
                            next_chunk->is_reachable |= 0b000010;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 25) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000001))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 0});
                            next_chunk->is_reachable |= 0b000001;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 26) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b001000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z, 3});
                            next_chunk->is_reachable |= 0b001000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 27) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000100))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z, 2});
                            next_chunk->is_reachable |= 0b000100;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 29) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b010000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z + 1, 4});
                            next_chunk->is_reachable |= 0b010000;
                        }
                    }
                }
            }
        } else {
            if (((current_chunk->reachability_code >> 30) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000010))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x - 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 1});
                            next_chunk->is_reachable |= 0b000010;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 31) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000001))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x + 1, local_chunk_coordinates.y, local_chunk_coordinates.z, 0});
                            next_chunk->is_reachable |= 0b000001;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 32) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b001000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y - 1, local_chunk_coordinates.z, 3});
                            next_chunk->is_reachable |= 0b001000;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 33) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b000100))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y + 1, local_chunk_coordinates.z, 2});
                            next_chunk->is_reachable |= 0b000100;
                        }
                    }
                }
            }

            if (((current_chunk->reachability_code >> 34) & 1)) {
                if (next_chunk = storage_.GetChunk(local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1)) {
                    if (next_chunk->is_visible && !((next_chunk->is_reachable & 0b100000))) {
                        if ((current_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (current_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (current_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (current_chunk->global_coordinates.z - starting_chunk_coordinates.z) <

                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) *
                            (next_chunk->global_coordinates.x - starting_chunk_coordinates.x) +
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) *
                            (next_chunk->global_coordinates.y - starting_chunk_coordinates.y) +
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z) *
                            (next_chunk->global_coordinates.z - starting_chunk_coordinates.z)) {
                        
                            ACCA_queue_.push({local_chunk_coordinates.x, local_chunk_coordinates.y, local_chunk_coordinates.z - 1, 5});
                            next_chunk->is_reachable |= 0b100000;
                        }
                    }
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
                if (storage_.chunks_[(y * storage_.sizes.z + z) * storage_.sizes.x + x]->is_reachable) {
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
