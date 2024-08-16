#include "ChunkStorage.h"

#include "../Exceptions/Exceptions.h"
#include "../Block/Block.h"

#include <chrono>
#include <random>

ChunkStorage::ChunkStorage(const glm::ivec3& sizes) : sizes(sizes) {
    chunk_count = sizes.x * sizes.y * sizes.z;

    voxels_ = new Voxel[chunk_count * Chunk::VOLUME];
    lightmaps_ = new uint16_t[chunk_count * Chunk::VOLUME]{};
    face_planes_ = new uint16_t[chunk_count * Chunk::DIRECTION_SIZE_P2 * Chunk::FACES_COUNT_PER_CUBE]{};
    chunks_ = new Chunk*[chunk_count];

    if (voxels_ == nullptr) {
        std::cout << "Bad alloc: voxels_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (face_planes_ == nullptr) {
        std::cout << "Bad alloc: face_planes_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (lightmaps_ == nullptr) {
        std::cout << "Bad alloc: lightmaps_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (chunks_ == nullptr) {
        std::cout << "Bad alloc: chunks_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    for (int i = 0, y = 0; y < sizes.y; ++y) {
        for (int z = 0; z < sizes.z; ++z) {
            for (int x = 0; x < sizes.x; ++x, ++i) {
                chunks_[i] = new Chunk({x, y, z},
                                       &voxels_[i * Chunk::VOLUME],
                                       &lightmaps_[i * Chunk::VOLUME],
                                       &face_planes_[i * Chunk::DIRECTION_SIZE_P2 * Chunk::FACES_COUNT_PER_CUBE],
                                       this);

                if (chunks_[i] == nullptr) {
                    std::cout << "Bad alloc: chunks_[i]" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
            }
        }
    }

    for (int i = 0; i < chunk_count; ++i) {
        chunks_[i]->CullingChunksJoints();
    }

    /*std::mt19937 generator{std::random_device{}()};
    std::uniform_int_distribution<> random(1, 128);

    for (int global_z = (rendering_center.z - sizes.z / 2) * Chunk::DEPTH + 1; global_z < (rendering_center.z + (sizes.z + 1) / 2) * Chunk::DEPTH - 1; ++global_z) {
        for (int global_x = (rendering_center.x - sizes.x / 2) * Chunk::WIDTH + 1; global_x < (rendering_center.x + (sizes.x + 1) / 2) * Chunk::WIDTH - 1; ++global_x) {
            for (int global_y = 0 + 1; global_y < sizes.y * Chunk::HEIGHT - 1; ++global_y) {
                Voxel* voxel = GetVoxel(global_x, global_y, global_z);

                if (voxel->id != 0) {
                    if (GetVoxel(global_x + 1, global_y, global_z)->id == 0 || GetVoxel(global_x, global_y + 1, global_z)->id == 0 ||
                        GetVoxel(global_x - 1, global_y, global_z)->id == 0 || GetVoxel(global_x, global_y - 1, global_z)->id == 0 ||
                        GetVoxel(global_x, global_y, global_z + 1)->id == 0 || GetVoxel(global_x, global_y, global_z - 1)->id == 0) {

                        int seed = random(generator);

                        if (seed == 126) {
                            voxel->id = 4;
                        } else if (seed == 127) {
                            voxel->id = 5;
                        } else if (seed == 128) {
                            voxel->id = 6;
                        }
                    }
                }
            }
        }
    }*/


    R = new LightProcessor(this, 0);
    G = new LightProcessor(this, 1);
    B = new LightProcessor(this, 2);
    S = new LightProcessor(this, 3);

    for (int i = 0; i < chunk_count; ++i) {
        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                for (int x = 0; x < Chunk::WIDTH; ++x) {
                    int global_x = x + chunks_[i]->global_coordinates.x * Chunk::WIDTH;
                    int global_y = y + chunks_[i]->global_coordinates.y * Chunk::DEPTH;
                    int global_z = z + chunks_[i]->global_coordinates.z * Chunk::HEIGHT;

                    if (chunks_[i]->voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id == 4) {
                        R->Add(global_x, global_y, global_z, 15);
                        G->Add(global_x, global_y, global_z, 0);
                        B->Add(global_x, global_y, global_z, 0);
                    } else if (chunks_[i]->voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id == 5) {
                        R->Add(global_x, global_y, global_z, 0);
                        G->Add(global_x, global_y, global_z, 15);
                        B->Add(global_x, global_y, global_z, 0);
                    } else if (chunks_[i]->voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id == 6) {
                        R->Add(global_x, global_y, global_z, 0);
                        G->Add(global_x, global_y, global_z, 0);
                        B->Add(global_x, global_y, global_z, 15);
                    } 
                }
            }
        }
    }

    for (int global_z = (rendering_center.z - sizes.z / 2) * Chunk::DEPTH; global_z < (rendering_center.z + (sizes.z + 1) / 2) * Chunk::DEPTH; ++global_z) {
        for (int global_x = (rendering_center.x - sizes.x / 2) * Chunk::WIDTH; global_x < (rendering_center.x + (sizes.x + 1) / 2) * Chunk::WIDTH; ++global_x) {
            for (int global_y = sizes.y * Chunk::HEIGHT - 1; global_y >= 0; --global_y) {
                Voxel* voxel = GetVoxel(global_x, global_y, global_z);

                if (voxel->id != 0) {
                    break;
                }

                Chunk* chunk = GetChunkByVoxel(global_x, global_y, global_z);

                chunk->lightmap_->SetS(global_x - chunk->global_coordinates.x * Chunk::WIDTH,
                                      global_y - chunk->global_coordinates.y * Chunk::HEIGHT,
                                      global_z - chunk->global_coordinates.z * Chunk::DEPTH,
                                      0xF);
            }
        }
    }

    for (int global_z = (rendering_center.z - sizes.z / 2) * Chunk::DEPTH; global_z < (rendering_center.z + (sizes.z + 1) / 2) * Chunk::DEPTH; ++global_z) {
        for (int global_x = (rendering_center.x - sizes.x / 2) * Chunk::WIDTH; global_x < (rendering_center.x + (sizes.x + 1) / 2) * Chunk::WIDTH; ++global_x) {
            for (int global_y = sizes.y * Chunk::HEIGHT - 1; global_y >= 0; --global_y) {
                Voxel* voxel = GetVoxel(global_x, global_y, global_z);

                if (voxel->id != 0) {
                    break;
                }

                if (GetLight(global_x - 1, global_y, global_z, 3) == 0 ||
                    GetLight(global_x + 1, global_y, global_z, 3) == 0 ||
                    GetLight(global_x, global_y - 1, global_z, 3) == 0 ||
                    GetLight(global_x, global_y + 1, global_z, 3) == 0 ||
                    GetLight(global_x, global_y, global_z - 1, 3) == 0 ||
                    GetLight(global_x, global_y, global_z + 1, 3) == 0) {
                    S->Add(global_x, global_y, global_z);
                }

                Chunk* chunk = GetChunkByVoxel(global_x, global_y, global_z);

                chunk->lightmap_->SetS(global_x - chunk->global_coordinates.x * Chunk::WIDTH,
                                      global_y - chunk->global_coordinates.y * Chunk::HEIGHT,
                                      global_z - chunk->global_coordinates.z * Chunk::DEPTH,
                                      0xF);
            }
        }
    }

    R->Process();
    G->Process();
    B->Process();
    S->Process();

    std::cout << static_cast<int>(GetLight(8, 100, 8, 0)) << ' ' << static_cast<int>(GetLight(8, 100, 8, 1)) << ' ' << static_cast<int>(GetLight(8, 100, 8, 2)) << ' ' << static_cast<int>(GetLight(8, 100, 8, 3)) << '\n';
    std::cout << static_cast<int>(GetLight(16, 16, 16, 0)) << ' ' << static_cast<int>(GetLight(16, 16, 16, 1)) << ' ' << static_cast<int>(GetLight(16, 16, 16, 2)) << ' ' << static_cast<int>(GetLight(16, 16, 16, 3)) << '\n';
    std::cout << static_cast<int>(GetLight(16, 3, 16, 0)) << ' ' << static_cast<int>(GetLight(16, 3, 16, 1)) << ' ' << static_cast<int>(GetLight(16, 3, 16, 2)) << ' ' << static_cast<int>(GetLight(16, 3, 16, 3)) << '\n';

}

ChunkStorage::~ChunkStorage() {
    delete[] voxels_;
    delete[] lightmaps_;
    delete[] face_planes_;

    for (int i = 0; i < chunk_count; ++i) {
        delete chunks_[i];
    }
    delete[] chunks_;
}

Voxel* ChunkStorage::GetVoxel(int x, int y, int z) const {
    const Chunk* chunk = GetChunkByVoxel(x, y, z);

    if (chunk) {
        x -= chunk->global_coordinates.x * Chunk::WIDTH;
        y -= chunk->global_coordinates.y * Chunk::HEIGHT;
        z -= chunk->global_coordinates.z * Chunk::DEPTH;

        return &chunk->voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x];
    }
    
    return nullptr;
}

void ChunkStorage::SetVoxel(int x, int y, int z, uint8_t block_id) {
    Chunk* chunk = GetChunkByVoxel(x, y, z);

    if (!chunk) {
        return;
    }

    x -= chunk->global_coordinates.x * Chunk::WIDTH;
    y -= chunk->global_coordinates.y * Chunk::HEIGHT;
    z -= chunk->global_coordinates.z * Chunk::DEPTH;


    Chunk* neighbouring_chunk;
    chunk->voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id = block_id;
    chunk->is_modified = true;

    if (block_id) {

        if (x == Chunk::WIDTH - 1) {
            if (!chunk->IsBlocked(Chunk::WIDTH, y, z)) {
                chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] |= (1 << y);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x + 1, chunk->local_coordinates.y, chunk->local_coordinates.z)) {
                    neighbouring_chunk->face_planes_[1][0 * Chunk::DEPTH + z] = neighbouring_chunk->face_planes_[1][0 * Chunk::DEPTH + z] & ~(1 << y);
                }
            }

            if (!chunk->IsBlocked(Chunk::WIDTH - 2, y, z)) {
                chunk->face_planes_[1][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] |= (1 << y);
            } else {
                chunk->face_planes_[0][(Chunk::WIDTH - 2) * Chunk::DEPTH + z] =
                    chunk->face_planes_[0][(Chunk::WIDTH - 2) * Chunk::DEPTH + z] & ~(1 << y);
            }
        } else if (x == 0) {
            if (!chunk->IsBlocked(-1, y, z)) {
                chunk->face_planes_[1][0 * Chunk::DEPTH + z] |= (1 << y);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x - 1, chunk->local_coordinates.y, chunk->local_coordinates.z)) {
                    neighbouring_chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] =
                        neighbouring_chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] & ~(1 << y);
                }
            }

            if (!chunk->IsBlocked(1, y, z)) {
                chunk->face_planes_[0][0 * Chunk::DEPTH + z] |= (1 << y);
            } else {
                chunk->face_planes_[1][1 * Chunk::DEPTH + z] = chunk->face_planes_[1][1 * Chunk::DEPTH + z] & ~(1 << y);
            }
        } else {
            if (!chunk->IsBlocked(x + 1, y, z)) {
                chunk->face_planes_[0][x * Chunk::DEPTH + z] |= (1 << y);
            } else {
                chunk->face_planes_[1][(x + 1) * Chunk::DEPTH + z] = chunk->face_planes_[1][(x + 1) * Chunk::DEPTH + z] & ~(1 << y);
            }

            if (!chunk->IsBlocked(x - 1, y, z)) {
                chunk->face_planes_[1][x * Chunk::DEPTH + z] |= (1 << y);
            } else {
                chunk->face_planes_[0][(x - 1) * Chunk::DEPTH + z] = chunk->face_planes_[0][(x - 1) * Chunk::DEPTH + z] & ~(1 << y);
            }
        }

        if (y == Chunk::HEIGHT - 1) {
            if (!chunk->IsBlocked(x, Chunk::HEIGHT, z)) {
                chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] |= (1 << z);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x, chunk->local_coordinates.y + 1, chunk->local_coordinates.z)) {
                    neighbouring_chunk->face_planes_[3][0 * Chunk::WIDTH + x] = neighbouring_chunk->face_planes_[3][0 * Chunk::WIDTH + x] & ~(1 << z);
                }
            }

            if (!chunk->IsBlocked(x, Chunk::HEIGHT - 2, z)) {
                chunk->face_planes_[3][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] |= (1 << z);
            } else {
                chunk->face_planes_[2][(Chunk::HEIGHT - 2) * Chunk::WIDTH + x] =
                    chunk->face_planes_[2][(Chunk::HEIGHT - 2) * Chunk::WIDTH + x] & ~(1 << z);
            }
        } else if (y == 0) {
            if (!chunk->IsBlocked(x, -1, z)) {
                chunk->face_planes_[3][0 * Chunk::WIDTH + x] |= (1 << z);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x, chunk->local_coordinates.y - 1, chunk->local_coordinates.z)) {
                    neighbouring_chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] =
                        neighbouring_chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] & ~(1 << z);
                }
            }

            if (!chunk->IsBlocked(x, 1, z)) {
                chunk->face_planes_[2][0 * Chunk::WIDTH + x] |= (1 << z);
            } else {
                chunk->face_planes_[3][1 * Chunk::WIDTH + x] = chunk->face_planes_[3][1 * Chunk::WIDTH + x] & ~(1 << z);
            }
        } else {
            if (!chunk->IsBlocked(x, y + 1, z)) {
                chunk->face_planes_[2][y * Chunk::WIDTH + x] |= (1 << z);
            } else {
                chunk->face_planes_[3][(y + 1) * Chunk::WIDTH + x] = chunk->face_planes_[3][(y + 1) * Chunk::WIDTH + x] & ~(1 << z);
            }

            if (!chunk->IsBlocked(x, y - 1, z)) {
                chunk->face_planes_[3][y * Chunk::WIDTH + x] |= (1 << z);
            } else {
                chunk->face_planes_[2][(y - 1) * Chunk::WIDTH + x] = chunk->face_planes_[2][(y - 1) * Chunk::WIDTH + x] & ~(1 << z);
            }
        }

        if (z == Chunk::DEPTH - 1) {
            if (!chunk->IsBlocked(x, y, Chunk::DEPTH)) {
                chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] |= (1 << x);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x, chunk->local_coordinates.y, chunk->local_coordinates.z + 1)) {
                    neighbouring_chunk->face_planes_[5][0 * Chunk::HEIGHT + y] =
                        neighbouring_chunk->face_planes_[5][0 * Chunk::HEIGHT + y] & ~(1 << x);
                }
            }

            if (!chunk->IsBlocked(x, y, Chunk::DEPTH - 2)) {
                chunk->face_planes_[5][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] |= (1 << x);
            } else {
                chunk->face_planes_[4][(Chunk::DEPTH - 2) * Chunk::HEIGHT + y] =
                    chunk->face_planes_[4][(Chunk::DEPTH - 2) * Chunk::HEIGHT + y] & ~(1 << x);
            }
        } else if (z == 0) {
            if (!chunk->IsBlocked(x, y, -1)) {
                chunk->face_planes_[5][0 * Chunk::HEIGHT + y] |= (1 << x);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x, chunk->local_coordinates.y, chunk->local_coordinates.z - 1)) {
                    neighbouring_chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] =
                        neighbouring_chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] & ~(1 << x);
                }
            }

            if (!chunk->IsBlocked(x, y, 1)) {
                chunk->face_planes_[4][0 * Chunk::HEIGHT + y] |= (1 << x);
            } else {
                chunk->face_planes_[5][1 * Chunk::HEIGHT + y] = chunk->face_planes_[5][1 * Chunk::HEIGHT + y] & ~(1 << x);
            }
        } else {
            if (!chunk->IsBlocked(x, y, z + 1)) {
                chunk->face_planes_[4][z * Chunk::HEIGHT + y] |= (1 << x);
            } else {
                chunk->face_planes_[5][(z + 1) * Chunk::HEIGHT + y] = chunk->face_planes_[5][(z + 1) * Chunk::HEIGHT + y] & ~(1 << x);
            }

            if (!chunk->IsBlocked(x, y, z - 1)) {
                chunk->face_planes_[5][z * Chunk::HEIGHT + y] |= (1 << x);
            } else {
                chunk->face_planes_[4][(z - 1) * Chunk::HEIGHT + y] = chunk->face_planes_[4][(z - 1) * Chunk::HEIGHT + y] & ~(1 << x);
            }
        }

    } else {

        if (x == Chunk::WIDTH - 1) {
            if (!chunk->IsBlocked(Chunk::WIDTH, y, z)) {
                chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] =
                    chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] & ~(1 << y);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x + 1, chunk->local_coordinates.y, chunk->local_coordinates.z)) {
                    neighbouring_chunk->face_planes_[1][0 * Chunk::DEPTH + z] |= (1 << y);
                }
            }

            if (!chunk->IsBlocked(Chunk::WIDTH - 2, y, z)) {
                chunk->face_planes_[1][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] =
                    chunk->face_planes_[1][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] & ~(1 << y);
            } else {
                chunk->face_planes_[0][(Chunk::WIDTH - 2) * Chunk::DEPTH + z] |= (1 << y);
            }
        } else if (x == 0) {
            if (!chunk->IsBlocked(-1, y, z)) {
                chunk->face_planes_[1][0 * Chunk::DEPTH + z] = chunk->face_planes_[1][0 * Chunk::DEPTH + z] & ~(1 << y);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x - 1, chunk->local_coordinates.y, chunk->local_coordinates.z)) {
                    neighbouring_chunk->face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] |= (1 << y);
                }
            }

            if (!chunk->IsBlocked(1, y, z)) {
                chunk->face_planes_[0][0 * Chunk::DEPTH + z] = chunk->face_planes_[0][0 * Chunk::DEPTH + z] & ~(1 << y);
            } else {
                chunk->face_planes_[1][1 * Chunk::DEPTH + z] |= (1 << y);
            }
        } else {
            if (!chunk->IsBlocked(x + 1, y, z)) {
                chunk->face_planes_[0][x * Chunk::DEPTH + z] = chunk->face_planes_[0][x * Chunk::DEPTH + z] & ~(1 << y);
            } else {
                chunk->face_planes_[1][(x + 1) * Chunk::DEPTH + z] |= (1 << y);
            }

            if (!chunk->IsBlocked(x - 1, y, z)) {
                chunk->face_planes_[1][x * Chunk::DEPTH + z] = chunk->face_planes_[1][x * Chunk::DEPTH + z] & ~(1 << y);
            } else {
                chunk->face_planes_[0][(x - 1) * Chunk::DEPTH + z] |= (1 << y);
            }
        }

        if (y == Chunk::HEIGHT - 1) {
            if (!chunk->IsBlocked(x, Chunk::HEIGHT, z)) {
                chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] =
                    chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] & ~(1 << z);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x, chunk->local_coordinates.y + 1, chunk->local_coordinates.z)) {
                    neighbouring_chunk->face_planes_[3][0 * Chunk::WIDTH + x] |= (1 << z);
                }
            }

            if (!chunk->IsBlocked(x, Chunk::HEIGHT - 2, z)) {
                chunk->face_planes_[3][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] =
                    chunk->face_planes_[3][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] & ~(1 << z);
            } else {
                chunk->face_planes_[2][(Chunk::HEIGHT - 2) * Chunk::WIDTH + x] |= (1 << z);
            }
        } else if (y == 0) {
            if (!chunk->IsBlocked(x, -1, z)) {
                chunk->face_planes_[3][0 * Chunk::WIDTH + x] = chunk->face_planes_[3][0 * Chunk::WIDTH + x] & ~(1 << z);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x, chunk->local_coordinates.y - 1, chunk->local_coordinates.z)) {
                    neighbouring_chunk->face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] |= (1 << z);
                }
            }

            if (!chunk->IsBlocked(x, 1, z)) {
                chunk->face_planes_[2][0 * Chunk::WIDTH + x] = chunk->face_planes_[2][0 * Chunk::WIDTH + x] & ~(1 << z);
            } else {
                chunk->face_planes_[3][1 * Chunk::WIDTH + x] |= (1 << z);
            }
        } else {
            if (!chunk->IsBlocked(x, y + 1, z)) {
                chunk->face_planes_[2][y * Chunk::WIDTH + x] = chunk->face_planes_[2][y * Chunk::WIDTH + x] & ~(1 << z);
            } else {
                chunk->face_planes_[3][(y + 1) * Chunk::WIDTH + x] |= (1 << z);
            }

            if (!chunk->IsBlocked(x, y - 1, z)) {
                chunk->face_planes_[3][y * Chunk::WIDTH + x] = chunk->face_planes_[3][y * Chunk::WIDTH + x] & ~(1 << z);
            } else {
                chunk->face_planes_[2][(y - 1) * Chunk::WIDTH + x] |= (1 << z);
            }
        }

        if (z == Chunk::DEPTH - 1) {
            if (!chunk->IsBlocked(x, y, Chunk::DEPTH)) {
                chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] =
                    chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] & ~(1 << x);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x, chunk->local_coordinates.y, chunk->local_coordinates.z + 1)) {
                    neighbouring_chunk->face_planes_[5][0 * Chunk::HEIGHT + y] |= (1 << x);
                }
            }

            if (!chunk->IsBlocked(x, y, Chunk::DEPTH - 2)) {
                chunk->face_planes_[5][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] =
                    chunk->face_planes_[5][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] & ~(1 << x);
            } else {
                chunk->face_planes_[4][(Chunk::DEPTH - 2) * Chunk::HEIGHT + y] |= (1 << x);
            }
        } else if (z == 0) {
            if (!chunk->IsBlocked(x, y, -1)) {
                chunk->face_planes_[5][0 * Chunk::HEIGHT + y] = chunk->face_planes_[5][0 * Chunk::HEIGHT + y] & ~(1 << x);
            } else {
                if (neighbouring_chunk = GetChunk(chunk->local_coordinates.x, chunk->local_coordinates.y, chunk->local_coordinates.z - 1)) {
                    neighbouring_chunk->face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] |= (1 << x);
                }
            }

            if (!chunk->IsBlocked(x, y, 1)) {
                chunk->face_planes_[4][0 * Chunk::HEIGHT + y] = chunk->face_planes_[4][0 * Chunk::HEIGHT + y] & ~(1 << x);
            } else {
                chunk->face_planes_[5][1 * Chunk::HEIGHT + y] |= (1 << x);
            }
        } else {
            if (!chunk->IsBlocked(x, y, z + 1)) {
                chunk->face_planes_[4][z * Chunk::HEIGHT + y] = chunk->face_planes_[4][z * Chunk::HEIGHT + y] & ~(1 << x);
            } else {
                chunk->face_planes_[5][(z + 1) * Chunk::HEIGHT + y] |= (1 << x);
            }

            if (!chunk->IsBlocked(x, y, z - 1)) {
                chunk->face_planes_[5][z * Chunk::HEIGHT + y] = chunk->face_planes_[5][z * Chunk::HEIGHT + y] & ~(1 << x);
            } else {
                chunk->face_planes_[4][(z - 1) * Chunk::HEIGHT + y] |= (1 << x);
            }
        }
    }
}

uint8_t ChunkStorage::GetLight(int x, int y, int z, int channel) const {
    const Chunk* chunk = GetChunkByVoxel(x, y, z);

    if (!chunk) {
        return 0;
    }

    x -= chunk->global_coordinates.x * Chunk::WIDTH;
    y -= chunk->global_coordinates.y * Chunk::HEIGHT;
    z -= chunk->global_coordinates.z * Chunk::DEPTH;

    return chunk->lightmap_->Get(x, y, z, channel);
}

inline Chunk* ChunkStorage::GetChunk(int x, int y, int z) const {
    if (x < 0 || y < 0 || z < 0 || x >= sizes.x || y >= sizes.y || z >= sizes.z) {
        return nullptr;
    }

    return chunks_[(y * sizes.z + z) * sizes.x + x];
}

Chunk* ChunkStorage::GetChunkByVoxel(int x, int y, int z) const {
    int chunk_local_x = (x < 0) ? (x + 1) / Chunk::WIDTH - 1 : x / Chunk::WIDTH;   // global
    int chunk_local_y = (y < 0) ? (y + 1) / Chunk::HEIGHT - 1 : y / Chunk::HEIGHT; // local
    int chunk_local_z = (z < 0) ? (z + 1) / Chunk::DEPTH - 1 : z / Chunk::DEPTH;   // global

    chunk_local_x += -rendering_center.x + sizes.x / 2; // local
    chunk_local_z += -rendering_center.z + sizes.z / 2; // local

    if (chunk_local_x < 0 || chunk_local_y < 0 || chunk_local_z < 0 || chunk_local_x >= sizes.x || chunk_local_y >= sizes.y ||
        chunk_local_z >= sizes.z) {
        return nullptr;
    }

    // chunks_->chunks_[y][z][x] := chunks_->chunks_[(y * sizes.z + z) * sizes.x + x]
    return chunks_[(chunk_local_y * sizes.z + chunk_local_z) * sizes.x + chunk_local_x];
}

Voxel* ChunkStorage::RayCast(const glm::vec3& a, const glm::vec3& dir, float max_ray_length, glm::vec3& end, glm::ivec3& normal, glm::ivec3& iend) const {
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

    int stepx = (dx > 0.0f) ? 1 : -1;
    int stepy = (dy > 0.0f) ? 1 : -1;
    int stepz = (dz > 0.0f) ? 1 : -1;

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

            normal.x = normal.y = normal.z = 0;
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
    normal.x = normal.y = normal.z = 0;
    return nullptr;
}

void ChunkStorage::CaptureTarget(LineBatch& line_batch, const Camera& camera) {
    glm::vec3 end;
    glm::ivec3 normal;
    glm::ivec3 iend;
    Voxel* voxel = RayCast(camera.position, camera.vector_front, 5.0f, end, normal, iend);
    if (voxel != nullptr) {
        line_batch.positions_[0] = {static_cast<float>(iend.x) - 0.001f, static_cast<float>(iend.y) - 0.001f, static_cast<float>(iend.z) - 0.001f};
        line_batch.positions_[1] = {static_cast<float>(iend.x) - 0.001f, static_cast<float>(iend.y) + 1.001f, static_cast<float>(iend.z) - 0.001f};
        line_batch.positions_[2] = {static_cast<float>(iend.x) - 0.001f, static_cast<float>(iend.y) + 1.001f, static_cast<float>(iend.z) + 1.001f};
        line_batch.positions_[3] = {static_cast<float>(iend.x) - 0.001f, static_cast<float>(iend.y) - 0.001f, static_cast<float>(iend.z) + 1.001f};
        line_batch.positions_[4] = {static_cast<float>(iend.x) + 1.001f, static_cast<float>(iend.y) - 0.001f, static_cast<float>(iend.z) - 0.001f};
        line_batch.positions_[5] = {static_cast<float>(iend.x) + 1.001f, static_cast<float>(iend.y) + 1.001f, static_cast<float>(iend.z) - 0.001f};
        line_batch.positions_[6] = {static_cast<float>(iend.x) + 1.001f, static_cast<float>(iend.y) + 1.001f, static_cast<float>(iend.z) + 1.001f};
        line_batch.positions_[7] = {static_cast<float>(iend.x) + 1.001f, static_cast<float>(iend.y) - 0.001f, static_cast<float>(iend.z) + 1.001f};

        line_batch.RewritePositionData();

        line_batch.draw_box = true;

        int x;
        int y;
        int z;

        if (Events::MouseIsClicked(GLFW_MOUSE_BUTTON_1)) {
            x = iend.x;
            y = iend.y;
            z = iend.z;

            SetVoxel(x, y, z, 0);

            R->Remove(x, y, z);
            G->Remove(x, y, z);
            B->Remove(x, y, z);

            R->Process();
            G->Process();
            B->Process();

            if (GetLight(x, y + 1, z, 3) == 0xF) {
                for (int i = y; i >= 0; --i) {
                    if (GetVoxel(x, i, z)->id != 0) {
                        break;
                    }
                    S->Add(x, i, z, 0xF);
                }
            }

            R->Add(x, y + 1, z); G->Add(x, y + 1, z); B->Add(x, y + 1, z); S->Add(x, y + 1, z);
            R->Add(x, y - 1, z); G->Add(x, y - 1, z); B->Add(x, y - 1, z); S->Add(x, y - 1, z);
            R->Add(x + 1, y, z); G->Add(x + 1, y, z); B->Add(x + 1, y, z); S->Add(x + 1, y, z);
            R->Add(x - 1, y, z); G->Add(x - 1, y, z); B->Add(x - 1, y, z); S->Add(x - 1, y, z);
            R->Add(x, y, z + 1); G->Add(x, y, z + 1); B->Add(x, y, z + 1); S->Add(x, y, z + 1);
            R->Add(x, y, z - 1); G->Add(x, y, z - 1); B->Add(x, y, z - 1); S->Add(x, y, z - 1);

            R->Process();
            G->Process();
            B->Process();
            S->Process();

        } else if (Events::MouseIsClicked(GLFW_MOUSE_BUTTON_2)) {
            x = iend.x + normal.x;
            y = iend.y + normal.y;
            z = iend.z + normal.z;

            SetVoxel(x, y, z, selected_block);

            R->Remove(x, y, z);
            G->Remove(x, y, z);
            B->Remove(x, y, z);
            S->Remove(x, y, z);
            for (int i = y - 1; i >= 0; --i) {
                S->Remove(x, i, z);

                if (i == 0 || GetVoxel(x, i - 1, z)->id != 0) {
                    break;
                }
            }
            R->Process();
            G->Process();
            B->Process();
            S->Process();

            if (selected_block >= 4) {
                R->Add(x, y, z, Blocks::blocks[selected_block].r);
                G->Add(x, y, z, Blocks::blocks[selected_block].g);
                B->Add(x, y, z, Blocks::blocks[selected_block].b);
                R->Process();
                G->Process();
                B->Process();
            }

        }
    } else {
        line_batch.draw_box = false;
    }
}

void ChunkStorage::UpdateChanges() {
    for (int i = 0; i < chunk_count; ++i) {
        if (chunks_[i]->is_modified) {
            auto start = std::chrono::high_resolution_clock::now();

            chunks_[i]->GreedyMesh();


            //std::cout << ((chunks_[i]->vertex_data[0] >> 46) & 0x7F) << ' ' << ((chunks_[i]->vertex_data[0] >> 39) & 0x7F) << ' '
            //          << ((chunks_[i]->vertex_data[0] >> 32) & 0x7F) << ' ' << ((chunks_[i]->vertex_data[0] >> 25) & 0x7F) << '\n';

            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            //std::cout << "Time taken by function: " << duration.count() << " microseconds\n";
            //std::cout << "Faces count: " << chunks_[i]->vertex_data_size / Chunk::VERTICES_COUNT_PER_SQUARE << '\n';
        }
    }
}
