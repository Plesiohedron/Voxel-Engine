#include "Chunks.h"

#include <chrono>

glm::ivec3 Chunks::storage_sizes;


Chunks::Chunks(const int radius, const glm::ivec3& center): rendering_radius(radius), rendering_center(center) {
    storage_sizes = {2 * radius - 1, CHUNK_COUNT_IN_HEIGHT, 2 * radius - 1};
    chunk_count = storage_sizes.x * storage_sizes.y * storage_sizes.z;

    models_.resize(chunk_count);
    chunks_ = new Chunk*[chunk_count];
    VAOs_ = new GL::SChunkVAO*[storage_sizes.x * storage_sizes.z];

    if (chunks_ == nullptr) {
        std::cout << "Bad alloc: chunks_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (VAOs_ == nullptr) {
        std::cout << "Bad alloc: VAOs_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    for (int i = 0, y = 0; y < storage_sizes.y; ++y) {
        for (int z = 0; z < storage_sizes.z; ++z) {
            for (int x = 0; x < storage_sizes.x; ++x, ++i) {
                chunks_[i] = new Chunk({x, y, z});
                models_[i] = glm::translate(glm::mat4(1.0f), glm::vec3(chunks_[i]->global_coordinates.x * Chunk::WIDTH,
                                                                       chunks_[i]->global_coordinates.y * Chunk::HEIGHT,
                                                                       chunks_[i]->global_coordinates.z * Chunk::DEPTH));

                if (chunks_[i] == nullptr) {
                    std::cout << "Bad alloc: chunks_[i]" << std::endl;
                    std::exit(EXIT_FAILURE);
                }

            }
        }
    }

    Chunk::chunk_storage_ = chunks_;
    for (int i = 0; i < chunk_count; ++i) {
        chunks_[i]->CullingChunksJoints();
    }

    uint32_t* unified_index_data = new uint32_t[UNIFIED_EBO_TRIANGLES_SIZE];

    if (unified_index_data == nullptr) {
        std::cout << "Bad alloc: unified_index_data" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    for (int i = 0, j = 0; i < UNIFIED_EBO_TRIANGLES_SIZE; i += Chunk::INDEXES_COUNT_PER_SQUARE, j += Chunk::VERTICES_COUNT_PER_SQUARE) {
        unified_index_data[i] = j + 0;
        unified_index_data[i + 1] = j + 1;
        unified_index_data[i + 2] = j + 2;
        unified_index_data[i + 3] = j + 2;
        unified_index_data[i + 4] = j + 3;
        unified_index_data[i + 5] = j + 0;
    }

    glGenBuffers(1, &unified_EBO_triangles);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unified_EBO_triangles);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * UNIFIED_EBO_TRIANGLES_SIZE, unified_index_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete[] unified_index_data;

    unified_index_data = new uint32_t[UNIFIED_EBO_LINES_SIZE];

    if (unified_index_data == nullptr) {
        std::cout << "Bad alloc: unified_index_data" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    for (int i = 0, j = 0; i < UNIFIED_EBO_LINES_SIZE; i += LINES_COUNT_PER_SQUARE, j += Chunk::VERTICES_COUNT_PER_SQUARE) {
        unified_index_data[i] = j + 0;
        unified_index_data[i + 1] = j + 1;
        unified_index_data[i + 2] = j + 1;
        unified_index_data[i + 3] = j + 2;
        unified_index_data[i + 4] = j + 2;
        unified_index_data[i + 5] = j + 3;
        unified_index_data[i + 6] = j + 3;
        unified_index_data[i + 7] = j + 0;
        unified_index_data[i + 8] = j + 0;
        unified_index_data[i + 9] = j + 2;
    }

    glGenBuffers(1, &unified_EBO_lines);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unified_EBO_lines);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * UNIFIED_EBO_LINES_SIZE, unified_index_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    delete[] unified_index_data;

    for (int i = 0, z = 0; z < storage_sizes.z; ++z) {
        for (int x = 0; x < storage_sizes.x; ++x, ++i) {
            VAOs_[i] = new GL::SChunkVAO{};

            if (VAOs_[i] == nullptr) {
                std::cout << "Bad alloc: VAOs_[i]" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            VAOs_[i]->Bind();
            VAOs_[i]->AllocateVBO(CHUNK_COUNT_IN_HEIGHT * Chunk::STARTING_VOXEL_FACES_CAPACITY * Chunk::VERTICES_COUNT_PER_SQUARE);
            //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unified_EBO_triangles);
        }
    }
    UnbindVAO();

    matrices_SSBO_.InitializeMatrices(models_, 0);


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
}

void Chunks::UnbindVAO() const {
    glBindVertexArray(0);
}

Voxel* Chunks::GetVoxel(int x, int y, int z) const {
    int chunk_local_x = (x < 0) ? (x + 1) / Chunk::WIDTH - 1 : x / Chunk::WIDTH;   // global
    int chunk_local_y = (y < 0) ? (y + 1) / Chunk::HEIGHT - 1 : y / Chunk::HEIGHT; // local
    int chunk_local_z = (z < 0) ? (z + 1) / Chunk::DEPTH - 1 : z / Chunk::DEPTH;   // global

    int voxel_local_x = x - chunk_local_x * Chunk::WIDTH;
    int voxel_local_y = y - chunk_local_y * Chunk::HEIGHT;
    int voxel_local_z = z - chunk_local_z * Chunk::DEPTH;

    chunk_local_x += -rendering_center.x + storage_sizes.x / 2;  // local
    chunk_local_z += -rendering_center.z + storage_sizes.z / 2;  // local

    if (chunk_local_x < 0 || chunk_local_y < 0 || chunk_local_z < 0 ||
        chunk_local_x >= storage_sizes.x || chunk_local_y >= storage_sizes.y || chunk_local_z >= storage_sizes.z) {
        return nullptr;
    }

    // chunks_->chunks_[y][z][x] := chunks_->chunks_[(y * storage_sizes.z + z) * storage_sizes.x + x]
    const Chunk* chunk = chunks_[(chunk_local_y * storage_sizes.z + chunk_local_z) * storage_sizes.x + chunk_local_x];

    return &chunk->voxels_[(voxel_local_y * Chunk::DEPTH + voxel_local_z) * Chunk::WIDTH + voxel_local_x];
}

inline Chunk* Chunks::GetChunk(int x, int y, int z) const {
    if (x < 0 || y < 0 || z < 0 || x >= storage_sizes.x || y >= storage_sizes.y || z >= storage_sizes.z) {
        return nullptr;
    }

    return chunks_[(y * storage_sizes.z + z) * storage_sizes.x + x];
}

void Chunks::SetVoxel(int x, int y, int z, uint8_t id) {
    int chunk_local_x = (x < 0) ? (x + 1) / Chunk::WIDTH - 1 : x / Chunk::WIDTH;    // global
    int chunk_local_y = (y < 0) ? (y + 1) / Chunk::HEIGHT - 1 : y / Chunk::HEIGHT;  // local
    int chunk_local_z = (z < 0) ? (z + 1) / Chunk::DEPTH - 1 : z / Chunk::DEPTH;    // global

    int voxel_local_x = x - chunk_local_x * Chunk::WIDTH;
    int voxel_local_y = y - chunk_local_y * Chunk::HEIGHT;
    int voxel_local_z = z - chunk_local_z * Chunk::DEPTH;

    chunk_local_x += -rendering_center.x + storage_sizes.x / 2;  // local
    chunk_local_z += -rendering_center.z + storage_sizes.z / 2;  // local

    if (chunk_local_x < 0 || chunk_local_y < 0 || chunk_local_z < 0 ||
        chunk_local_x >= storage_sizes.x || chunk_local_y >= storage_sizes.y || chunk_local_z >= storage_sizes.z) {
        return;
    }

    // chunks_->chunks_[y][z][x] := chunks_->chunks_[(y * storage_sizes.z + z) * storage_sizes.x + x]
    Chunk* chunk = chunks_[(chunk_local_y * storage_sizes.z + chunk_local_z) * storage_sizes.x + chunk_local_x];
    Chunk* neighbouring_chunk;
    chunk->voxels_[(voxel_local_y * Chunk::DEPTH + voxel_local_z) * Chunk::WIDTH + voxel_local_x].id = id;
    chunk->is_modified = true;

    if (id) {

        if (voxel_local_x == Chunk::WIDTH - 1) {
            if (!chunk->IsBlocked(Chunk::WIDTH, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y, chunk_local_z)) {
                    neighbouring_chunk->face_planes_[1][0 * Chunk::DEPTH + voxel_local_z] =
                        neighbouring_chunk->face_planes_[1][0 * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
                }
            }

            if (!chunk->IsBlocked(Chunk::WIDTH - 2, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[1][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            } else {
                chunk->face_planes_[0][(Chunk::WIDTH - 2) * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[0][(Chunk::WIDTH - 2) * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            }
        } else if (voxel_local_x == 0) {
            if (!chunk->IsBlocked(-1, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[1][0 * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y, chunk_local_z)) {
                    neighbouring_chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] =
                        neighbouring_chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
                }
            }

            if (!chunk->IsBlocked(1, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[0][0 * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            } else {
                chunk->face_planes_[1][1 * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[1][1 * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            }
        } else {
            if (!chunk->IsBlocked(voxel_local_x + 1, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[0][voxel_local_x * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            } else {
                chunk->face_planes_[1][(voxel_local_x + 1) * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[1][(voxel_local_x + 1) * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            }

            if (!chunk->IsBlocked(voxel_local_x - 1, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[1][voxel_local_x * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            } else {
                chunk->face_planes_[0][(voxel_local_x - 1) * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[0][(voxel_local_x - 1) * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            }
        }

        if (voxel_local_y == Chunk::HEIGHT - 1) {
            if (!chunk->IsBlocked(voxel_local_x, Chunk::HEIGHT, voxel_local_z)) {
                chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y + 1, chunk_local_z)) {
                    neighbouring_chunk->face_planes_[3][0 * Chunk::WIDTH + voxel_local_x] =
                        neighbouring_chunk->face_planes_[3][0 * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
                }
            }

            if (!chunk->IsBlocked(voxel_local_x, Chunk::HEIGHT - 2, voxel_local_z)) {
                chunk->face_planes_[3][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            } else {
                chunk->face_planes_[2][(Chunk::HEIGHT - 2) * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[2][(Chunk::HEIGHT - 2) * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            }
        } else if (voxel_local_y == 0) {
            if (!chunk->IsBlocked(voxel_local_x, -1, voxel_local_z)) {
                chunk->face_planes_[3][0 * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y - 1, chunk_local_z)) {
                    neighbouring_chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] =
                        neighbouring_chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
                }
            }

            if (!chunk->IsBlocked(voxel_local_x, 1, voxel_local_z)) {
                chunk->face_planes_[2][0 * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            } else {
                chunk->face_planes_[3][1 * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[3][1 * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            }
        } else {
            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y + 1, voxel_local_z)) {
                chunk->face_planes_[2][voxel_local_y * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            } else {
                chunk->face_planes_[3][(voxel_local_y + 1) * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[3][(voxel_local_y + 1) * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            }

            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y - 1, voxel_local_z)) {
                chunk->face_planes_[3][voxel_local_y * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            } else {
                chunk->face_planes_[2][(voxel_local_y - 1) * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[2][(voxel_local_y - 1) * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            }
        }

        if (voxel_local_z == Chunk::DEPTH - 1) {
            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, Chunk::DEPTH)) {
                chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y, chunk_local_z + 1)) {
                    neighbouring_chunk->face_planes_[5][0 * Chunk::HEIGHT + voxel_local_y] =
                        neighbouring_chunk->face_planes_[5][0 * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
                }
            }

            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, Chunk::DEPTH - 2)) {
                chunk->face_planes_[5][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            } else {
                chunk->face_planes_[4][(Chunk::DEPTH - 2) * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[4][(Chunk::DEPTH - 2) * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            }
        } else if (voxel_local_z == 0) {
            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, -1)) {
                chunk->face_planes_[5][0 * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y, chunk_local_z - 1)) {
                    neighbouring_chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] =
                        neighbouring_chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
                }
            }

            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, 1)) {
                chunk->face_planes_[4][0 * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            } else {
                chunk->face_planes_[5][1 * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[5][1 * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            }
        } else {
            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, voxel_local_z + 1)) {
                chunk->face_planes_[4][voxel_local_z * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            } else {
                chunk->face_planes_[5][(voxel_local_z + 1) * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[5][(voxel_local_z + 1) * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            }

            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, voxel_local_z - 1)) {
                chunk->face_planes_[5][voxel_local_z * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            } else {
                chunk->face_planes_[4][(voxel_local_z - 1) * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[4][(voxel_local_z - 1) * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            }
        }

    } else {
    
        if (voxel_local_x == Chunk::WIDTH - 1) {
            if (!chunk->IsBlocked(Chunk::WIDTH, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y, chunk_local_z)) {
                    neighbouring_chunk->face_planes_[1][0 * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
                }
            }

            if (!chunk->IsBlocked(Chunk::WIDTH - 2, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[1][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[1][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            } else {
                chunk->face_planes_[0][(Chunk::WIDTH - 2) * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            }
        } else if (voxel_local_x == 0) {
            if (!chunk->IsBlocked(-1, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[1][0 * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[1][0 * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y, chunk_local_z)) {
                    neighbouring_chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
                }
            }

            if (!chunk->IsBlocked(1, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[0][0 * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[0][0 * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            } else {
                chunk->face_planes_[1][1 * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            }
        } else {
            if (!chunk->IsBlocked(voxel_local_x + 1, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[0][voxel_local_x * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[0][voxel_local_x * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            } else {
                chunk->face_planes_[1][(voxel_local_x + 1) * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            }

            if (!chunk->IsBlocked(voxel_local_x - 1, voxel_local_y, voxel_local_z)) {
                chunk->face_planes_[1][voxel_local_x * Chunk::DEPTH + voxel_local_z] =
                    chunk->face_planes_[1][voxel_local_x * Chunk::DEPTH + voxel_local_z] & ~(1 << voxel_local_y);
            } else {
                chunk->face_planes_[0][(voxel_local_x - 1) * Chunk::DEPTH + voxel_local_z] |= (1 << voxel_local_y);
            }
        }

        if (voxel_local_y == Chunk::HEIGHT - 1) {
            if (!chunk->IsBlocked(voxel_local_x, Chunk::HEIGHT, voxel_local_z)) {
                chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y + 1, chunk_local_z)) {
                    neighbouring_chunk->face_planes_[3][0 * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
                }
            }

            if (!chunk->IsBlocked(voxel_local_x, Chunk::HEIGHT - 2, voxel_local_z)) {
                chunk->face_planes_[3][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[3][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            } else {
                chunk->face_planes_[2][(Chunk::HEIGHT - 2) * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            }
        } else if (voxel_local_y == 0) {
            if (!chunk->IsBlocked(voxel_local_x, -1, voxel_local_z)) {
                chunk->face_planes_[3][0 * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[3][0 * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y - 1, chunk_local_z)) {
                    neighbouring_chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
                }
            }

            if (!chunk->IsBlocked(voxel_local_x, 1, voxel_local_z)) {
                chunk->face_planes_[2][0 * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[2][0 * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            } else {
                chunk->face_planes_[3][1 * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            }
        } else {
            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y + 1, voxel_local_z)) {
                chunk->face_planes_[2][voxel_local_y * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[2][voxel_local_y * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            } else {
                chunk->face_planes_[3][(voxel_local_y + 1) * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            }

            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y - 1, voxel_local_z)) {
                chunk->face_planes_[3][voxel_local_y * Chunk::WIDTH + voxel_local_x] =
                    chunk->face_planes_[3][voxel_local_y * Chunk::WIDTH + voxel_local_x] & ~(1 << voxel_local_z);
            } else {
                chunk->face_planes_[2][(voxel_local_y - 1) * Chunk::WIDTH + voxel_local_x] |= (1 << voxel_local_z);
            }
        }

        if (voxel_local_z == Chunk::DEPTH - 1) {
            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, Chunk::DEPTH)) {
                chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y, chunk_local_z + 1)) {
                    neighbouring_chunk->face_planes_[5][0 * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
                }
            }

            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, Chunk::DEPTH - 2)) {
                chunk->face_planes_[5][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[5][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            } else {
                chunk->face_planes_[4][(Chunk::DEPTH - 2) * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            }
        } else if (voxel_local_z == 0) {
            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, -1)) {
                chunk->face_planes_[5][0 * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[5][0 * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            } else {
                if (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y, chunk_local_z - 1)) {
                    neighbouring_chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
                }
            }

            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, 1)) {
                chunk->face_planes_[4][0 * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[4][0 * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            } else {
                chunk->face_planes_[5][1 * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            }
        } else {
            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, voxel_local_z + 1)) {
                chunk->face_planes_[4][voxel_local_z * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[4][voxel_local_z * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            } else {
                chunk->face_planes_[5][(voxel_local_z + 1) * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            }

            if (!chunk->IsBlocked(voxel_local_x, voxel_local_y, voxel_local_z - 1)) {
                chunk->face_planes_[5][voxel_local_z * Chunk::HEIGHT + voxel_local_y] =
                    chunk->face_planes_[5][voxel_local_z * Chunk::HEIGHT + voxel_local_y] & ~(1 << voxel_local_x);
            } else {
                chunk->face_planes_[4][(voxel_local_z - 1) * Chunk::HEIGHT + voxel_local_y] |= (1 << voxel_local_x);
            }
        }

    }

    if (voxel_local_x == 0 && voxel_local_y == 0 && voxel_local_z == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y - 1, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == 0 && voxel_local_y == 0 && voxel_local_z == Chunk::DEPTH - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y - 1, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == 0 && voxel_local_y == Chunk::HEIGHT - 1 && voxel_local_z == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y + 1, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == 0 && voxel_local_y == Chunk::HEIGHT - 1 && voxel_local_z == Chunk::DEPTH - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y + 1, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && voxel_local_y == 0 && voxel_local_z == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y - 1, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && voxel_local_y == 0 && voxel_local_z == Chunk::DEPTH - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y - 1, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && voxel_local_y == Chunk::HEIGHT - 1 && voxel_local_z == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y + 1, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && voxel_local_y == Chunk::HEIGHT - 1 && voxel_local_z == Chunk::DEPTH - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y + 1, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }

    if (voxel_local_x == 0 && voxel_local_y == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y - 1, chunk_local_z))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == 0 && voxel_local_y == Chunk::HEIGHT - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y + 1, chunk_local_z))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && voxel_local_y == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y - 1, chunk_local_z))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && voxel_local_y == Chunk::HEIGHT - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y + 1, chunk_local_z))) {
        neighbouring_chunk->is_modified = true;
    }

    if (voxel_local_x == 0 && voxel_local_z == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == 0 && voxel_local_z == Chunk::DEPTH - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && voxel_local_z == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && voxel_local_z == Chunk::DEPTH - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }

    if (voxel_local_y == 0 && voxel_local_z == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y - 1, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_y == 0 && voxel_local_z == Chunk::DEPTH - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y - 1, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_y == Chunk::HEIGHT - 1 && voxel_local_z == 0 &&
        (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y + 1, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_y == Chunk::HEIGHT - 1 && voxel_local_z == Chunk::DEPTH - 1 &&
        (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y + 1, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }

    if (voxel_local_x == 0 && (neighbouring_chunk = GetChunk(chunk_local_x - 1, chunk_local_y, chunk_local_z))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_x == Chunk::WIDTH - 1 && (neighbouring_chunk = GetChunk(chunk_local_x + 1, chunk_local_y, chunk_local_z))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_y == 0 && (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y - 1, chunk_local_z))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_y == Chunk::HEIGHT - 1 && (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y + 1, chunk_local_z))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_z == 0 && (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y, chunk_local_z - 1))) {
        neighbouring_chunk->is_modified = true;
    }
    if (voxel_local_z == Chunk::DEPTH - 1 && (neighbouring_chunk = GetChunk(chunk_local_x, chunk_local_y, chunk_local_z + 1))) {
        neighbouring_chunk->is_modified = true;
    }
}

Voxel* Chunks::RayCast(glm::vec3 a, glm::vec3 dir, float max_ray_length, glm::vec3& end, glm::vec3& normal, glm::vec3& iend) {
    float px = a.x;
    float py = a.y;
    float pz = a.z;

    float dx = dir.x;
    float dy = dir.y;
    float dz = dir.z;

    float t = 0.0f;
    int ix = floor(px);
    int iy = floor(py);
    int iz = floor(pz);

    float stepx = (dx > 0.0f) ? 1.0f : -1.0f;
    float stepy = (dy > 0.0f) ? 1.0f : -1.0f;
    float stepz = (dz > 0.0f) ? 1.0f : -1.0f;

    float infinity = std::numeric_limits<float>::infinity();

    float txDelta = (dx == 0.0f) ? infinity : abs(1.0f / dx);
    float tyDelta = (dy == 0.0f) ? infinity : abs(1.0f / dy);
    float tzDelta = (dz == 0.0f) ? infinity : abs(1.0f / dz);

    float xdist = (stepx > 0) ? (ix + 1 - px) : (px - ix);
    float ydist = (stepy > 0) ? (iy + 1 - py) : (py - iy);
    float zdist = (stepz > 0) ? (iz + 1 - pz) : (pz - iz);

    float txMax = (txDelta < infinity) ? txDelta * xdist : infinity;
    float tyMax = (tyDelta < infinity) ? tyDelta * ydist : infinity;
    float tzMax = (tzDelta < infinity) ? tzDelta * zdist : infinity;

    int stepped_index = -1;

    while (t <= max_ray_length) {
        Voxel* voxel = GetVoxel(ix, iy, iz);
        if (voxel == nullptr || voxel->id) {
            end.x = px + t * dx;
            end.y = py + t * dy;
            end.z = pz + t * dz;

            iend.x = ix;
            iend.y = iy;
            iend.z = iz;

            normal.x = normal.y = normal.z = 0.0f;
            if (stepped_index == 0)
                normal.x = -stepx;
            if (stepped_index == 1)
                normal.y = -stepy;
            if (stepped_index == 2)
                normal.z = -stepz;
            return voxel;
        }
        if (txMax < tyMax) {
            if (txMax < tzMax) {
                ix += stepx;
                t = txMax;
                txMax += txDelta;
                stepped_index = 0;
            } else {
                iz += stepz;
                t = tzMax;
                tzMax += tzDelta;
                stepped_index = 2;
            }
        } else {
            if (tyMax < tzMax) {
                iy += stepy;
                t = tyMax;
                tyMax += tyDelta;
                stepped_index = 1;
            } else {
                iz += stepz;
                t = tzMax;
                tzMax += tzDelta;
                stepped_index = 2;
            }
        }
    }
    iend.x = ix;
    iend.y = iy;
    iend.z = iz;

    end.x = px + t * dx;
    end.y = py + t * dy;
    end.z = pz + t * dz;
    normal.x = normal.y = normal.z = 0.0f;
    return nullptr;
}

void Chunks::PollUpdates() {

    for (int i = 0; i < chunk_count; ++i) {
        if (chunks_[i]->is_modified) {
            auto start = std::chrono::high_resolution_clock::now();

            chunks_[i]->GreedyMesh();

            //std::cout << ((chunks_[i]->vertex_data[0] >> 52) & 0xF) << ' ' << ((chunks_[i]->vertex_data[0] >> 48) & 0xF) << ' ' << ((chunks_[i]->vertex_data[0] >> 44) & 0xF) << ' '
            //          << ((chunks_[i]->vertex_data[0] >> 40) & 0xF) << ' ' << ((chunks_[i]->vertex_data[0] >> 32) & 0xFF) << ' ' << ((chunks_[i]->vertex_data[0] >> 30) & 0x3) << ' '
            //          << ((chunks_[i]->vertex_data[0] >> 24) & 0x3F) << ' ' << ((chunks_[i]->vertex_data[0] >> 18) & 0x3F) << ' ' << ((chunks_[i]->vertex_data[0] >> 12) & 0x3F) << ' '
            //          << ((chunks_[i]->vertex_data[0] >> 6) & 0x3F) << ' ' << (chunks_[i]->vertex_data[0] & 0x3F) << '\n';

            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Time taken by function: " << duration.count() << " microseconds\n";
            std::cout << "Faces count: " << chunks_[i]->vertex_data_size / Chunk::VERTICES_COUNT_PER_SQUARE << '\n';
        }
    }

    for (int i = 0, z = 0; z < Chunks::storage_sizes.z; ++z) {
        for (int x = 0; x < Chunks::storage_sizes.x; ++x, ++i) {

            int SChunkVAO_capacity = 0;
            bool is_SChunk_modified = false;
            bool is_Chunk_capacity_changed = false;
            for (int y = 0; y < Chunks::storage_sizes.y; ++y) {
                // chunks_->chunks_[y][z][x] := chunks_->chunks_[(y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x]
                const Chunk* chunk = chunks_[(y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x];

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
                    for (int y = 0; y < Chunks::storage_sizes.y; ++y) {
                        Chunk* chunk = chunks_[(y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x];

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
                    glBufferData(GL_ARRAY_BUFFER, sizeof(uint64_t) * SChunkVAO_capacity, nullptr, GL_DYNAMIC_DRAW);
                    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, 0, nullptr);
                    glEnableVertexAttribArray(0);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);

                    int unmodified_SChunk_offset = 0;
                    int modified_SChunk_offset = 0;
                    for (int y = 0; y < Chunks::storage_sizes.y; ++y) {
                        Chunk* chunk = chunks_[(y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x];

                        if (chunk->is_modified) {
                            glBindBuffer(GL_ARRAY_BUFFER, temp_VBO);
                            glBufferSubData(GL_ARRAY_BUFFER, sizeof(uint64_t) * modified_SChunk_offset,
                                            sizeof(uint64_t) * chunk->vertex_data_size, chunk->vertex_data);
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

                            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
                                                sizeof(uint64_t) * unmodified_SChunk_offset,
                                                sizeof(uint64_t) * modified_SChunk_offset,
                                                sizeof(uint64_t) * chunk->voxel_faces_size * Chunk::VERTICES_COUNT_PER_SQUARE);

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
    UnbindVAO();

}

void Chunks::Draw(const Camera& camera) const {
    shader_->Use();
    shader_->UniformMatrix(uniform_projection_loc_, camera.GetProjection());
    shader_->UniformMatrix(uniform_view_loc_, camera.GetView());

    glActiveTexture(GL_TEXTURE0);
    texture_atlas_->Bind();
    shader_->UniformTexture(uniform_texture_loc_, 0);

    for (int i = 0, z = 0; z < Chunks::storage_sizes.z; ++z) {
        for (int x = 0; x < Chunks::storage_sizes.x; ++x, ++i) {
            VAOs_[i]->Bind();
            
            if (rendering_mode_ == GL_TRIANGLES) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unified_EBO_triangles);

                int offset = 0;
                for (int y = 0; y < Chunks::storage_sizes.y; ++y) {
                    shader_->UniformInt(uniform_model_index_loc_, (y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x);
                    glDrawElementsBaseVertex(rendering_mode_,
                                             chunks_[(y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x]->voxel_faces_size *
                                                 Chunk::INDEXES_COUNT_PER_SQUARE,
                                             GL_UNSIGNED_INT, nullptr, Chunk::VERTICES_COUNT_PER_SQUARE * offset);
                    offset += chunks_[(y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x]->voxel_faces_capacity;
                }
            } else if (rendering_mode_ == GL_LINES) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unified_EBO_lines);

                int offset = 0;
                for (int y = 0; y < Chunks::storage_sizes.y; ++y) {
                    shader_->UniformInt(uniform_model_index_loc_, (y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x);
                    glDrawElementsBaseVertex(rendering_mode_,
                                             chunks_[(y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x]->voxel_faces_size *
                                                 LINES_COUNT_PER_SQUARE,
                                             GL_UNSIGNED_INT, nullptr, Chunk::VERTICES_COUNT_PER_SQUARE * offset);
                    offset += chunks_[(y * Chunks::storage_sizes.z + z) * Chunks::storage_sizes.x + x]->voxel_faces_capacity;
                }
            }
        }
    }
    UnbindVAO();
}

Chunks::~Chunks() {
    glDeleteBuffers(1, &unified_EBO_triangles);
    glDeleteBuffers(1, &unified_EBO_lines);

    for (int i = 0; i < chunk_count; ++i) {
        delete chunks_[i];
    }

    for (int i = 0; i < storage_sizes.x * storage_sizes.z; ++i) {
        delete VAOs_[i];
    }

    delete[] VAOs_;
    delete[] chunks_;
}
