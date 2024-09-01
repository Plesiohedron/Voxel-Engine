#include "Chunk.h"
#include "../Block/Block.h"
#include "../Exceptions/Exceptions.h"

#include <immintrin.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <bitset>

#define ADD_FACE(l0, l1, l2, l3, texture_id, plane, row, bit, w, h) if (direction == 0) {\
PushBack({l0, l1, l2, l3, (static_cast<uint32_t>(texture_id) << 25) | ((plane + 1) << 20) | (bit << 15) | (row << 10) | (h << 5) | w});\
PushBack({l0, l1, l2, l3, (static_cast<uint32_t>(texture_id) << 25) | ((plane + 1) << 20) | ((bit + w) << 15) | (row << 10) | (h << 5) | 0});\
PushBack({l0, l1, l2, l3, (static_cast<uint32_t>(texture_id) << 25) | ((plane + 1) << 20) | ((bit + w) << 15) | ((row + h) << 10) | (0 << 5) | 0});\
PushBack({l0, l1, l2, l3, (static_cast<uint32_t>(texture_id) << 25) | ((plane + 1) << 20) | (bit << 15) | ((row + h) << 10) | (0 << 5) | w});\
}\
else if (direction == 1) {\
    PushBack({l3, l2, l1, l0, (static_cast<uint32_t>(texture_id) << 25) | (plane << 20) | (bit << 15) | ((row + h) << 10) | (h << 5) | w});\
    PushBack({l3, l2, l1, l0, (static_cast<uint32_t>(texture_id) << 25) | (plane << 20) | ((bit + w) << 15) | ((row + h) << 10) | (h << 5) | 0});\
    PushBack({l3, l2, l1, l0, (static_cast<uint32_t>(texture_id) << 25) | (plane << 20) | ((bit + w) << 15) | (row << 10) | (0 << 5) | 0});\
    PushBack({l3, l2, l1, l0, (static_cast<uint32_t>(texture_id) << 25) | (plane << 20) | (bit << 15) | (row << 10) | (0 << 5) | w});\
}\
else if (direction == 2) {\
    PushBack({l0, l3, l2, l1, (static_cast<uint32_t>(texture_id) << 25) | (row << 20) | ((plane + 1) << 15) | (bit << 10) | (0 << 5) | 0});\
    PushBack({l0, l3, l2, l1, (static_cast<uint32_t>(texture_id) << 25) | (row << 20) | ((plane + 1) << 15) | ((bit + w) << 10) | (0 << 5) | w});\
    PushBack(\
        {l0, l3, l2, l1, (static_cast<uint32_t>(texture_id) << 25) | ((row + h) << 20) | ((plane + 1) << 15) | ((bit + w) << 10) | (h << 5) | w});\
    PushBack({l0, l3, l2, l1, (static_cast<uint32_t>(texture_id) << 25) | ((row + h) << 20) | ((plane + 1) << 15) | (bit << 10) | (h << 5) | 0});\
}\
else if (direction == 3) {\
    PushBack({l1, l2, l3, l0, (static_cast<uint32_t>(texture_id) << 25) | ((row + h) << 20) | (plane << 15) | (bit << 10) | (h << 5) | w});\
    PushBack({l1, l2, l3, l0, (static_cast<uint32_t>(texture_id) << 25) | ((row + h) << 20) | (plane << 15) | ((bit + w) << 10) | (h << 5) | 0});\
    PushBack({l1, l2, l3, l0, (static_cast<uint32_t>(texture_id) << 25) | (row << 20) | (plane << 15) | ((bit + w) << 10) | (0 << 5) | 0});\
    PushBack({l1, l2, l3, l0, (static_cast<uint32_t>(texture_id) << 25) | (row << 20) | (plane << 15) | (bit << 10) | (0 << 5) | w});\
}\
else if (direction == 4) {\
    PushBack({l1, l0, l3, l2, (static_cast<uint32_t>(texture_id) << 25) | (bit << 20) | ((row + h) << 15) | ((plane + 1) << 10) | (0 << 5) | 0});\
    PushBack({l1, l0, l3, l2, (static_cast<uint32_t>(texture_id) << 25) | (bit << 20) | (row << 15) | ((plane + 1) << 10) | (0 << 5) | h});\
    PushBack({l1, l0, l3, l2, (static_cast<uint32_t>(texture_id) << 25) | ((bit + w) << 20) | (row << 15) | ((plane + 1) << 10) | (w << 5) | h});\
    PushBack(\
        {l1, l0, l3, l2, (static_cast<uint32_t>(texture_id) << 25) | ((bit + w) << 20) | ((row + h) << 15) | ((plane + 1) << 10) | (w << 5) | 0});\
}\
else {\
    PushBack({l0, l1, l2, l3, (static_cast<uint32_t>(texture_id) << 25) | (bit << 20) | (row << 15) | (plane << 10) | (w << 5) | h});\
    PushBack({l0, l1, l2, l3, (static_cast<uint32_t>(texture_id) << 25) | (bit << 20) | ((row + h) << 15) | (plane << 10) | (w << 5) | 0});\
    PushBack({l0, l1, l2, l3, (static_cast<uint32_t>(texture_id) << 25) | ((bit + w) << 20) | ((row + h) << 15) | (plane << 10) | (0 << 5) | 0});\
    PushBack({l0, l1, l2, l3, (static_cast<uint32_t>(texture_id) << 25) | ((bit + w) << 20) | (row << 15) | (plane << 10) | (0 << 5) | h});\
}\


#define QUEUE_STEP if (coordinates.x + 1 < Chunk::WIDTH &&\
                       !((field[(coordinates.x + 1) * Chunk::HEIGHT + coordinates.y] >> coordinates.z) & 1) &&\
                       Blocks::blocks[voxels_[(coordinates.y * Chunk::DEPTH + coordinates.z) * Chunk::WIDTH + coordinates.x + 1].id].is_transparent) {\
                       queue[queue_size] = {coordinates.x + 1, coordinates.y, coordinates.z};\
                       field[(coordinates.x + 1) * Chunk::HEIGHT + coordinates.y] |= (1 << coordinates.z);\
\
                       ++queue_size;\
                    }\
                    if (coordinates.x - 1 >= 0 &&\
                        !((field[(coordinates.x - 1) * Chunk::HEIGHT + coordinates.y] >> coordinates.z) & 1) &&\
                        Blocks::blocks[voxels_[(coordinates.y * Chunk::DEPTH + coordinates.z) * Chunk::WIDTH + coordinates.x - 1].id].is_transparent) {\
                        queue[queue_size] = {coordinates.x - 1, coordinates.y, coordinates.z};\
                        field[(coordinates.x - 1) * Chunk::HEIGHT + coordinates.y] |= (1 << coordinates.z);\
\
                        ++queue_size;\
                    }\
\
                    if (coordinates.y + 1 < Chunk::HEIGHT &&\
                        !((field[coordinates.x * Chunk::HEIGHT + coordinates.y + 1] >> coordinates.z) & 1) &&\
                        Blocks::blocks[voxels_[((coordinates.y + 1) * Chunk::DEPTH + coordinates.z) * Chunk::WIDTH + coordinates.x].id].is_transparent) {\
                        queue[queue_size] = {coordinates.x, coordinates.y + 1, coordinates.z};\
                        field[coordinates.x * Chunk::HEIGHT + coordinates.y + 1] |= (1 << coordinates.z);\
\
                        ++queue_size;\
                    }\
                    if (coordinates.y - 1 >= 0 &&\
                        !((field[coordinates.x * Chunk::HEIGHT + coordinates.y - 1] >> coordinates.z) & 1) &&\
                        Blocks::blocks[voxels_[((coordinates.y - 1) * Chunk::DEPTH + coordinates.z) * Chunk::WIDTH + coordinates.x].id].is_transparent) {\
                        queue[queue_size] = {coordinates.x, coordinates.y - 1, coordinates.z};\
                        field[coordinates.x * Chunk::HEIGHT + coordinates.y - 1] |= (1 << coordinates.z);\
\
                        ++queue_size;\
                    }\
\
                    if (coordinates.z + 1 < Chunk::DEPTH &&\
                        !((field[coordinates.x * Chunk::HEIGHT + coordinates.y] >> (coordinates.z + 1)) & 1) &&\
                        Blocks::blocks[voxels_[(coordinates.y * Chunk::DEPTH + (coordinates.z + 1)) * Chunk::WIDTH + coordinates.x].id].is_transparent) {\
                        queue[queue_size] = {coordinates.x, coordinates.y, coordinates.z + 1};\
                        field[coordinates.x * Chunk::HEIGHT + coordinates.y] |= (1 << (coordinates.z + 1));\
\
                        ++queue_size;\
                    }\
                    if (coordinates.z - 1 >= 0 &&\
                        !((field[coordinates.x * Chunk::HEIGHT + coordinates.y] >> (coordinates.z - 1)) & 1) &&\
                        Blocks::blocks[voxels_[(coordinates.y * Chunk::DEPTH + (coordinates.z - 1)) * Chunk::WIDTH + coordinates.x].id].is_transparent) {\
                        queue[queue_size] = {coordinates.x, coordinates.y, coordinates.z - 1};\
                        field[coordinates.x * Chunk::HEIGHT + coordinates.y] |= (1 << (coordinates.z - 1));\
\
                        ++queue_size;\
                    }\

Chunk::Chunk(const glm::ivec3& coordinates, Voxel* voxels, uint16_t* lightmap, uint16_t* face_planes, const ChunkStorage* chunk_storage) {
    global_coordinates = {coordinates.x - chunk_storage->sizes.x / 2, coordinates.y, coordinates.z - chunk_storage->sizes.z / 2};
    local_coordinates = {coordinates.x, coordinates.y, coordinates.z};
    
    voxels_ = voxels;
    lightmap_ = new Lightmap{lightmap};
    for (int i = 0; i < Chunk::FACES_COUNT_PER_CUBE; ++i) {
        face_planes_[i] = &face_planes[DIRECTION_SIZE_P2 * i];
    }
    chunk_storage_ = chunk_storage;

    vertex_data = new Vertex[vertex_data_capacity];

    if (vertex_data == nullptr) {
        std::cout << "Bad alloc: vertex_data" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    alignas(32) uint16_t X_rows[Chunk::HEIGHT][Chunk::DEPTH]{};
    alignas(32) uint16_t Y_rows[Chunk::DEPTH][Chunk::WIDTH]{};
    alignas(32) uint16_t Z_rows[Chunk::WIDTH][Chunk::HEIGHT]{};

    for (int y = Chunk::HEIGHT - 1; y >= 0; --y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                int global_x = x + global_coordinates.x * Chunk::WIDTH;
                int global_y = y + global_coordinates.y * Chunk::DEPTH;
                int global_z = z + global_coordinates.z * Chunk::HEIGHT;

                //uint8_t block_id = 0;
                //uint8_t block_id = glm::perlin(glm::vec3(global_x * 0.0125f, global_y * 0.0125f, global_z * 0.0125f)) > 0.1f;
                //uint8_t block_id = global_y <= std::sin(0.1 * global_x) * 10;
                uint8_t block_id = global_y <= std::sin(0.1 * global_x) * 10 + std::cos(0.1 * global_z) * 10;
                //uint8_t block_id = global_y <= 10;
                if (global_y <= 2) {
                    block_id = 3;
                }

                //if (global_x < 3 || global_x > 7 || global_z < 3 || global_z > 7) {
                //    block_id = 0;
                //}

                //uint8_t block_id = 0 + (std::rand() % 2);

                //if ((global_x + global_y + global_z) % 2 && global_y < 16) {
                //    block_id = 1;
                //}

                if (block_id) {
                    X_rows[y][z] |= (1 << x);
                    Y_rows[z][x] |= (1 << y);
                    Z_rows[x][y] |= (1 << z);
                }

                // voxels[y][z][x] := voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]
                voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id = block_id;
            }
        }
    }

    Culling(X_rows, Y_rows, Z_rows);
    CalculateReachabilityCode();
}

void Chunk::CalculateReachabilityCode() {
    glm::ivec3* queue = chunk_storage_->reachability_field_queue;
    uint16_t* field = chunk_storage_->reachability_field;
    std::fill(field, field + Chunk::DIRECTION_SIZE_P2, 0);

    int queue_size = 0;
    int queue_index = 0;

    reachability_code = 0;
    uint64_t side_reachability_code = 0;

    bool flag = false;

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            if (!((field[0 * Chunk::HEIGHT + y] >> z) & 1) &&
                Blocks::blocks[voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + 0].id].is_transparent) {
                queue[queue_size] = {0, y, z};
                field[0 * Chunk::HEIGHT + y] |= (1 << z);

                ++queue_size;

                side_reachability_code = (1ull << 0);
                while (queue_index != queue_size) {
                    glm::ivec3 coordinates = queue[queue_index];

                    ++queue_index;

                    if (coordinates.x == Chunk::WIDTH - 1) {
                        side_reachability_code |= (1ull << 1);
                        side_reachability_code |= (1ull << 6);

                        side_reachability_code |= (1ull << 7);
                    }

                    if (coordinates.y == 0) {
                        side_reachability_code |= (1ull << 2);
                        side_reachability_code |= (1ull << 12);

                        side_reachability_code |= (1ull << 14);
                    } else if (coordinates.y == Chunk::HEIGHT - 1) {
                        side_reachability_code |= (1ull << 3);
                        side_reachability_code |= (1ull << 18);

                        side_reachability_code |= (1ull << 21);
                    }

                    if (coordinates.z == 0) {
                        side_reachability_code |= (1ull << 4);
                        side_reachability_code |= (1ull << 24);

                        side_reachability_code |= (1ull << 28);
                    } else if (coordinates.z == Chunk::DEPTH - 1) {
                        side_reachability_code |= (1ull << 5);
                        side_reachability_code |= (1ull << 30);

                        side_reachability_code |= (1ull << 35);
                    }

                    if ((side_reachability_code & 0b111111) == 0b111111) {
                        flag = true;
                        break;
                    }

                    QUEUE_STEP;
                }
            }

            if (flag) {
                break;
            }
        }

        if (flag) {
            break;
        }
    }

    if (flag) {
        reachability_code = 0b111111'111111'111111'111111'111111'111111;
    } else {
        reachability_code |= side_reachability_code;

        for (int i = 1; i < 5; ++i) {
            if (((side_reachability_code >> i) & 1)) {
                for (int j = i + 1; j < 6; ++j) {
                    if (((side_reachability_code >> j) & 1)) {
                        reachability_code |= (1ull << (6 * i + j));
                        reachability_code |= (1ull << (6 * j + i));
                    }
                }
            }
        }

        side_reachability_code = 0;

        queue_size = 0;
        queue_index = 0;

        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                if (!((field[(Chunk::WIDTH - 1) * Chunk::HEIGHT + y] >> z) & 1) &&
                    Blocks::blocks[voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + (Chunk::WIDTH - 1)].id].is_transparent) {
                    queue[queue_size] = {Chunk::WIDTH - 1, y, z};
                    field[(Chunk::WIDTH - 1) * Chunk::HEIGHT + y] |= (1 << z);

                    ++queue_size;

                    side_reachability_code = (1ull << 7);
                    while (queue_index != queue_size) {
                        glm::ivec3 coordinates = queue[queue_index];
                        ++queue_index;

                        if (coordinates.y == 0) {
                            side_reachability_code |= (1ull << 8);
                            side_reachability_code |= (1ull << 13);

                            side_reachability_code |= (1ull << 14);
                        } else if (coordinates.y == Chunk::HEIGHT - 1) {
                            side_reachability_code |= (1ull << 9);
                            side_reachability_code |= (1ull << 19);

                            side_reachability_code |= (1ull << 21);
                        }

                        if (coordinates.z == 0) {
                            side_reachability_code |= (1ull << 10);
                            side_reachability_code |= (1ull << 25);

                            side_reachability_code |= (1ull << 28);
                        } else if (coordinates.z == Chunk::DEPTH - 1) {
                            side_reachability_code |= (1ull << 11);
                            side_reachability_code |= (1ull << 31);

                            side_reachability_code |= (1ull << 35);
                        }

                        QUEUE_STEP;
                    }
                }
            }
        }

        reachability_code |= side_reachability_code;

        side_reachability_code >>= 6;

        for (int i = 2; i < 5; ++i) {
            if (((side_reachability_code >> i) & 1)) {
                for (int j = i + 1; j < 6; ++j) {
                    if (((side_reachability_code >> j) & 1)) {
                        reachability_code |= (1ull << (6 * i + j));
                        reachability_code |= (1ull << (6 * j + i));
                    }
                }
            }
        }

        side_reachability_code = 0;

        queue_size = 0;
        queue_index = 0;

        for (int x = 0; x < Chunk::WIDTH; ++x) {
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                if (!((field[x * Chunk::HEIGHT + 0] >> z) & 1) &&
                    Blocks::blocks[voxels_[(0 * Chunk::DEPTH + z) * Chunk::WIDTH + x].id].is_transparent) {
                    queue[queue_size] = {x, 0, z};
                    field[x * Chunk::HEIGHT + 0] |= (1 << z);

                    ++queue_size;

                    side_reachability_code = (1ull << 14);
                    while (queue_index != queue_size) {
                        glm::ivec3 coordinates = queue[queue_index];
                        ++queue_index;

                        if (coordinates.y == Chunk::HEIGHT - 1) {
                            side_reachability_code |= (1ull << 15);
                            side_reachability_code |= (1ull << 20);

                            side_reachability_code |= (1ull << 21);
                        }

                        if (coordinates.z == 0) {
                            side_reachability_code |= (1ull << 16);
                            side_reachability_code |= (1ull << 26);

                            side_reachability_code |= (1ull << 28);
                        } else if (coordinates.z == Chunk::DEPTH - 1) {
                            side_reachability_code |= (1ull << 17);
                            side_reachability_code |= (1ull << 32);

                            side_reachability_code |= (1ull << 35);
                        }

                        QUEUE_STEP;
                    }
                }
            }
        }
        reachability_code |= side_reachability_code;

        side_reachability_code >>= 12;

        for (int i = 3; i < 5; ++i) {
            if (((side_reachability_code >> i) & 1)) {
                for (int j = i + 1; j < 6; ++j) {
                    if (((side_reachability_code >> j) & 1)) {
                        reachability_code |= (1ull << (6 * i + j));
                        reachability_code |= (1ull << (6 * j + i));
                    }
                }
            }
        }

        side_reachability_code = 0;

        queue_size = 0;
        queue_index = 0;

        for (int x = 0; x < Chunk::WIDTH; ++x) {
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                if (!((field[x * Chunk::HEIGHT + Chunk::HEIGHT - 1] >> z) & 1) &&
                    Blocks::blocks[voxels_[((Chunk::HEIGHT - 1) * Chunk::DEPTH + z) * Chunk::WIDTH + x].id].is_transparent) {
                    queue[queue_size] = {x, Chunk::HEIGHT - 1, z};
                    field[x * Chunk::HEIGHT + (Chunk::HEIGHT - 1)] |= (1 << z);

                    ++queue_size;

                    side_reachability_code = (1ull << 21);
                    while (queue_index != queue_size) {
                        glm::ivec3 coordinates = queue[queue_index];
                        ++queue_index;

                        if (coordinates.z == 0) {
                            side_reachability_code |= (1ull << 22);
                            side_reachability_code |= (1ull << 27);

                            side_reachability_code |= (1ull << 28);
                        } else if (coordinates.z == Chunk::DEPTH - 1) {
                            side_reachability_code |= (1ull << 23);
                            side_reachability_code |= (1ull << 33);

                            side_reachability_code |= (1ull << 35);
                        }

                        QUEUE_STEP;
                    }
                }
            }
        }

        reachability_code |= side_reachability_code;

        side_reachability_code >>= 18;

        for (int i = 4; i < 5; ++i) {
            if (((side_reachability_code >> i) & 1)) {
                for (int j = i + 1; j < 6; ++j) {
                    if (((side_reachability_code >> j) & 1)) {
                        reachability_code |= (1ull << (6 * i + j));
                        reachability_code |= (1ull << (6 * j + i));
                    }
                }
            }
        }

        queue_size = 0;
        queue_index = 0;

        for (int x = 0; x < Chunk::WIDTH; ++x) {
            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                if (!((field[x * Chunk::HEIGHT + y] >> 0) & 1) &&
                    Blocks::blocks[voxels_[(y * Chunk::DEPTH + 0) * Chunk::WIDTH + x].id].is_transparent) {
                    queue[queue_size] = {x, y, 0};
                    field[x * Chunk::HEIGHT + y] |= (1 << 0);

                    ++queue_size;

                    reachability_code |= (1ull << 28);
                    while (queue_index != queue_size) {
                        glm::ivec3 coordinates = queue[queue_index];
                        ++queue_index;

                        if (coordinates.z == Chunk::DEPTH - 1) {
                            reachability_code |= (1ull << 24);
                            reachability_code |= (1ull << 29);

                            reachability_code |= (1ull << 35);
                        }

                        QUEUE_STEP;
                    }
                }
            }
        }

        for (int x = 0; x < Chunk::WIDTH; ++x) {
            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                if (!((field[x * Chunk::HEIGHT + y] >> (Chunk::DEPTH - 1)) & 1) &&
                    Blocks::blocks[voxels_[(y * Chunk::DEPTH + Chunk::DEPTH - 1) * Chunk::WIDTH + x].id].is_transparent) {

                    reachability_code |= (1ull << 35);
                    return;
                }
            }
        }
    }
}

void Chunk::Culling(uint16_t (&X_rows)[Chunk::HEIGHT][Chunk::DEPTH],
                    uint16_t (&Y_rows)[Chunk::DEPTH][Chunk::WIDTH],
                    uint16_t (&Z_rows)[Chunk::WIDTH][Chunk::HEIGHT]) {

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        __m256i X_plane = _mm256_load_si256(reinterpret_cast<const __m256i*>(&X_rows[y][0]));

        __m256i forward_faces = _mm256_and_si256(X_plane, _mm256_andnot_si256(_mm256_srli_epi16(X_plane, 1), _mm256_set1_epi16(0xFFFF)));
        __m256i back_faces = _mm256_and_si256(X_plane, _mm256_andnot_si256(_mm256_slli_epi16(X_plane, 1), _mm256_set1_epi16(0xFFFF)));

        alignas(32) uint16_t forward_faces_plane[16];
        alignas(32) uint16_t back_faces_plane[16];
        _mm256_store_si256(reinterpret_cast<__m256i*>(forward_faces_plane), forward_faces);
        _mm256_store_si256(reinterpret_cast<__m256i*>(back_faces_plane), back_faces);

        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                face_planes_[0][x * Chunk::DEPTH + z] |= ((forward_faces_plane[z] >> x) & 1) << y;
                face_planes_[1][x * Chunk::DEPTH + z] |= ((back_faces_plane[z] >> x) & 1) << y;
            }
        }
    }

    for (int z = 0; z < Chunk::DEPTH; ++z) {
        __m256i Y_plane = _mm256_load_si256(reinterpret_cast<const __m256i*>(&Y_rows[z][0]));

        __m256i up_faces = _mm256_and_si256(Y_plane, _mm256_andnot_si256(_mm256_srli_epi16(Y_plane, 1), _mm256_set1_epi16(0xFFFF)));
        __m256i down_faces = _mm256_and_si256(Y_plane, _mm256_andnot_si256(_mm256_slli_epi16(Y_plane, 1), _mm256_set1_epi16(0xFFFF)));

        alignas(32) uint16_t up_faces_plane[16];
        alignas(32) uint16_t down_faces_plane[16];
        _mm256_store_si256(reinterpret_cast<__m256i*>(up_faces_plane), up_faces);
        _mm256_store_si256(reinterpret_cast<__m256i*>(down_faces_plane), down_faces);

        for (int x = 0; x < Chunk::WIDTH; ++x) {
            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                face_planes_[2][y * Chunk::WIDTH + x] |= ((up_faces_plane[x] >> y) & 1) << z;
                face_planes_[3][y * Chunk::WIDTH + x] |= ((down_faces_plane[x] >> y) & 1) << z;
            }
        }
    }

    for (int x = 0; x < Chunk::WIDTH; ++x) {
        __m256i Z_plane = _mm256_load_si256(reinterpret_cast<const __m256i*>(&Z_rows[x][0]));

        __m256i right_faces = _mm256_and_si256(Z_plane, _mm256_andnot_si256(_mm256_srli_epi16(Z_plane, 1), _mm256_set1_epi16(0xFFFF)));
        __m256i left_faces = _mm256_and_si256(Z_plane, _mm256_andnot_si256(_mm256_slli_epi16(Z_plane, 1), _mm256_set1_epi16(0xFFFF)));

        alignas(32) uint16_t right_faces_plane[16];
        alignas(32) uint16_t left_faces_plane[16];
        _mm256_store_si256(reinterpret_cast<__m256i*>(right_faces_plane), right_faces);
        _mm256_store_si256(reinterpret_cast<__m256i*>(left_faces_plane), left_faces);

        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                face_planes_[4][z * Chunk::HEIGHT + y] |= ((right_faces_plane[y] >> z) & 1) << x;
                face_planes_[5][z * Chunk::HEIGHT + y] |= ((left_faces_plane[y] >> z) & 1) << x;
            }
        }
    }
}

void Chunk::CullingChunksJoints() {
    uint16_t row_mask1;
    uint16_t row_mask2;


    for (int z = 0; z < Chunk::DEPTH; ++z) {
        row_mask1 = 0;
        row_mask2 = 0;
        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            row_mask1 |= (IsBlocked(-1, y, z) << y);
            row_mask2 |= (IsBlocked(Chunk::WIDTH, y, z) << y);
        }

        face_planes_[0][(Chunk::WIDTH - 1) * Chunk::DEPTH + z] &= ~row_mask2;
        face_planes_[1][0 * Chunk::DEPTH + z] &= ~row_mask1;
    }


    for (int x = 0; x < Chunk::WIDTH; ++x) {
        row_mask1 = 0;
        row_mask2 = 0;
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            row_mask1 |= (IsBlocked(x, -1, z) << z);
            row_mask2 |= (IsBlocked(x, Chunk::HEIGHT, z) << z);
        }

        face_planes_[2][(Chunk::HEIGHT - 1) * Chunk::WIDTH + x] &= ~row_mask2;
        face_planes_[3][0 * Chunk::WIDTH + x] &= ~row_mask1;
    }


    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        row_mask1 = 0;
        row_mask2 = 0;
        for (int x = 0; x < Chunk::WIDTH; ++x) {
            row_mask1 |= (IsBlocked(x, y, -1) << x);
            row_mask2 |= (IsBlocked(x, y, Chunk::DEPTH) << x);
        }

        face_planes_[4][(Chunk::DEPTH - 1) * Chunk::HEIGHT + y] &= ~row_mask2;
        face_planes_[5][0 * Chunk::HEIGHT + y] &= ~row_mask1;
    }
}

bool Chunk::IsBlocked(int x, int y, int z) const {
    if (0 <= x && x < Chunk::WIDTH && 0 <= y && y < Chunk::HEIGHT && 0 <= z && z < Chunk::DEPTH) {
        return voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id;
    } else {
        int X = local_coordinates.x;
        int Y = local_coordinates.y;
        int Z = local_coordinates.z;

        if (x < 0) {
            --X;
            x = Chunk::WIDTH - 1;
        } else if (x == Chunk::WIDTH) {
            ++X;
            x = 0;
        }

        if (y < 0) {
            --Y;
            y = Chunk::HEIGHT - 1;
        } else if (y == Chunk::HEIGHT) {
            ++Y;
            y = 0;
        }

        if (z < 0) {
            --Z;
            z = Chunk::DEPTH - 1;
        } else if (z == Chunk::DEPTH) {
            ++Z;
            z = 0;
        }

        if (0 <= X && X < chunk_storage_->sizes.x && 0 <= Y && Y < chunk_storage_->sizes.y && 0 <= Z && Z < chunk_storage_->sizes.z) {
            return chunk_storage_->chunks_[(Y * chunk_storage_->sizes.z + Z) * chunk_storage_->sizes.x + X]
                   ->voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id;
        }

    }

    return true;
}

uint8_t Chunk::GetLight(int x, int y, int z, int channel) const {
    if (0 <= x && x < Chunk::WIDTH && 0 <= y && y < Chunk::HEIGHT && 0 <= z && z < Chunk::DEPTH) {
        return lightmap_->Get(x, y, z, channel);
    } else {
        int X = local_coordinates.x;
        int Y = local_coordinates.y;
        int Z = local_coordinates.z;

        if (x < 0) {
            --X;
            x = Chunk::WIDTH - 1;
        } else if (x == Chunk::WIDTH) {
            ++X;
            x = 0;
        }

        if (y < 0) {
            --Y;
            y = Chunk::HEIGHT - 1;
        } else if (y == Chunk::HEIGHT) {
            ++Y;
            y = 0;
        }

        if (z < 0) {
            --Z;
            z = Chunk::DEPTH - 1;
        } else if (z == Chunk::DEPTH) {
            ++Z;
            z = 0;
        }

        if (0 <= X && X < chunk_storage_->sizes.x && 0 <= Y && Y < chunk_storage_->sizes.y && 0 <= Z && Z < chunk_storage_->sizes.z) {
            return chunk_storage_->chunks_[(Y * chunk_storage_->sizes.z + Z) * chunk_storage_->sizes.x + X]->lightmap_->Get(x, y, z, channel);
        }
    }

    return 0;
}

uint32_t Chunk::Light(int x, int y, int z, int direction, int vertex) const {
    uint8_t r, g, b, s;

    if (direction < 2) {
        if (vertex == 0) {
            r = GetLight(x, y,     z,     0) +
                GetLight(x, y - 1, z - 1, 0) +
                GetLight(x, y - 1, z,     0) +
                GetLight(x, y,     z - 1, 0);

            g = GetLight(x, y,     z,     1) +
                GetLight(x, y - 1, z - 1, 1) +
                GetLight(x, y - 1, z,     1) +
                GetLight(x, y,     z - 1, 1);

            b = GetLight(x, y,     z,     2) +
                GetLight(x, y - 1, z - 1, 2) +
                GetLight(x, y - 1, z,     2) +
                GetLight(x, y,     z - 1, 2);

            s = GetLight(x, y,     z,     3) +
                GetLight(x, y - 1, z - 1, 3) +
                GetLight(x, y - 1, z,     3) +
                GetLight(x, y,     z - 1, 3);
        } else if (vertex == 1) {
            r = GetLight(x, y,     z,     0) +
                GetLight(x, y,     z - 1, 0) +
                GetLight(x, y + 1, z - 1, 0) +
                GetLight(x, y + 1, z,     0);

            g = GetLight(x, y,     z,     1) +
                GetLight(x, y,     z - 1, 1) +
                GetLight(x, y + 1, z - 1, 1) +
                GetLight(x, y + 1, z,     1);

            b = GetLight(x, y,     z,     2) +
                GetLight(x, y,     z - 1, 2) +
                GetLight(x, y + 1, z - 1, 2) +
                GetLight(x, y + 1, z,     2);

            s = GetLight(x, y,     z,     3) +
                GetLight(x, y,     z - 1, 3) +
                GetLight(x, y + 1, z - 1, 3) +
                GetLight(x, y + 1, z,     3);
        } else if (vertex == 2) {
            r = GetLight(x, y,     z,     0) +
                GetLight(x, y + 1, z,     0) +
                GetLight(x, y + 1, z + 1, 0) +
                GetLight(x, y, z + 1,     0);

            g = GetLight(x, y,     z,     1) +
                GetLight(x, y + 1, z,     1) +
                GetLight(x, y + 1, z + 1, 1) +
                GetLight(x, y, z + 1,     1);

            b = GetLight(x, y,     z,     2) +
                GetLight(x, y + 1, z,     2) +
                GetLight(x, y + 1, z + 1, 2) +
                GetLight(x, y, z + 1,     2);

            s = GetLight(x, y,     z,     3) +
                GetLight(x, y + 1, z,     3) +
                GetLight(x, y + 1, z + 1, 3) +
                GetLight(x, y, z + 1,     3);
        } else {  // if vertex == 3
            r = GetLight(x, y,     z,     0) +
                GetLight(x, y - 1, z,     0) +
                GetLight(x, y,     z + 1, 0) +
                GetLight(x, y - 1, z + 1, 0);

            g = GetLight(x, y,     z,     1) +
                GetLight(x, y - 1, z,     1) +
                GetLight(x, y,     z + 1, 1) +
                GetLight(x, y - 1, z + 1, 1);

            b = GetLight(x, y,     z,     2) +
                GetLight(x, y - 1, z,     2) +
                GetLight(x, y,     z + 1, 2) +
                GetLight(x, y - 1, z + 1, 2);

            s = GetLight(x, y,     z,     3) +
                GetLight(x, y - 1, z,     3) +
                GetLight(x, y,     z + 1, 3) +
                GetLight(x, y - 1, z + 1, 3);
        }
    } else if (direction < 4) {
        if (vertex == 0) {
            r = GetLight(x,     y, z,     0) +
                GetLight(x - 1, y, z - 1, 0) +
                GetLight(x,     y, z - 1, 0) +
                GetLight(x - 1, y, z,     0);

            g = GetLight(x,     y, z,     1) +
                GetLight(x - 1, y, z - 1, 1) +
                GetLight(x,     y, z - 1, 1) +
                GetLight(x - 1, y, z,     1);

            b = GetLight(x,     y, z,     2) +
                GetLight(x - 1, y, z - 1, 2) +
                GetLight(x,     y, z - 1, 2) +
                GetLight(x - 1, y, z,     2);

            s = GetLight(x,     y, z,     3) +
                GetLight(x - 1, y, z - 1, 3) +
                GetLight(x,     y, z - 1, 3) +
                GetLight(x - 1, y, z,     3);
        } else if (vertex == 1) {
            r = GetLight(x,     y, z,     0) +
                GetLight(x,     y, z - 1, 0) +
                GetLight(x + 1, y, z - 1, 0) +
                GetLight(x + 1, y, z,     0);

            g = GetLight(x,     y, z,     1) +
                GetLight(x,     y, z - 1, 1) +
                GetLight(x + 1, y, z - 1, 1) +
                GetLight(x + 1, y, z,     1);

            b = GetLight(x,     y, z,     2) +
                GetLight(x,     y, z - 1, 2) +
                GetLight(x + 1, y, z - 1, 2) +
                GetLight(x + 1, y, z,     2);

            s = GetLight(x,     y, z,     3) +
                GetLight(x,     y, z - 1, 3) +
                GetLight(x + 1, y, z - 1, 3) +
                GetLight(x + 1, y, z,     3);
        } else if (vertex == 2) {
            r = GetLight(x,     y, z,     0) +
                GetLight(x + 1, y, z,     0) +
                GetLight(x + 1, y, z + 1, 0) +
                GetLight(x,     y, z + 1, 0);

            g = GetLight(x,     y, z,     1) +
                GetLight(x + 1, y, z,     1) +
                GetLight(x + 1, y, z + 1, 1) +
                GetLight(x,     y, z + 1, 1);

            b = GetLight(x,     y, z,     2) +
                GetLight(x + 1, y, z,     2) +
                GetLight(x + 1, y, z + 1, 2) +
                GetLight(x,     y, z + 1, 2);

            s = GetLight(x,     y, z,     3) +
                GetLight(x + 1, y, z,     3) +
                GetLight(x + 1, y, z + 1, 3) +
                GetLight(x,     y, z + 1, 3);
        } else {  // if vertex == 3
            r = GetLight(x,     y, z,     0) +
                GetLight(x - 1, y, z,     0) +
                GetLight(x,     y, z + 1, 0) +
                GetLight(x - 1, y, z + 1, 0);

            g = GetLight(x,     y, z,     1) +
                GetLight(x - 1, y, z,     1) +
                GetLight(x,     y, z + 1, 1) +
                GetLight(x - 1, y, z + 1, 1);

            b = GetLight(x,     y, z,     2) +
                GetLight(x - 1, y, z,     2) +
                GetLight(x,     y, z + 1, 2) +
                GetLight(x - 1, y, z + 1, 2);

            s = GetLight(x,     y, z,     3) +
                GetLight(x - 1, y, z,     3) +
                GetLight(x,     y, z + 1, 3) +
                GetLight(x - 1, y, z + 1, 3);
        }
    } else {  // if (direction < 6)
        if (vertex == 0) {
            r = GetLight(x,     y,     z, 0) +
                GetLight(x - 1, y - 1, z, 0) +
                GetLight(x - 1, y,     z, 0) +
                GetLight(x,     y - 1, z, 0);

            g = GetLight(x,     y,     z, 1) +
                GetLight(x - 1, y - 1, z, 1) +
                GetLight(x - 1, y,     z, 1) +
                GetLight(x,     y - 1, z, 1);

            b = GetLight(x,     y,     z, 2) +
                GetLight(x - 1, y - 1, z, 2) +
                GetLight(x - 1, y,     z, 2) +
                GetLight(x,     y - 1, z, 2);

            s = GetLight(x,     y,     z, 3) +
                GetLight(x - 1, y - 1, z, 3) +
                GetLight(x - 1, y,     z, 3) +
                GetLight(x,     y - 1, z, 3);
        } else if (vertex == 1) {
            r = GetLight(x,     y,     z, 0) +
                GetLight(x - 1, y,     z, 0) +
                GetLight(x - 1, y + 1, z, 0) +
                GetLight(x,     y + 1, z, 0);

            g = GetLight(x,     y,     z, 1) +
                GetLight(x - 1, y,     z, 1) +
                GetLight(x - 1, y + 1, z, 1) +
                GetLight(x,     y + 1, z, 1);

            b = GetLight(x,     y,     z, 2) +
                GetLight(x - 1, y,     z, 2) +
                GetLight(x - 1, y + 1, z, 2) +
                GetLight(x,     y + 1, z, 2);

            s = GetLight(x,     y,     z, 3) +
                GetLight(x - 1, y,     z, 3) +
                GetLight(x - 1, y + 1, z, 3) +
                GetLight(x,     y + 1, z, 3);
        } else if (vertex == 2) {
            r = GetLight(x,     y,     z, 0) +
                GetLight(x,     y + 1, z, 0) +
                GetLight(x + 1, y + 1, z, 0) +
                GetLight(x + 1, y,     z, 0);

            g = GetLight(x,     y,     z, 1) +
                GetLight(x,     y + 1, z, 1) +
                GetLight(x + 1, y + 1, z, 1) +
                GetLight(x + 1, y,     z, 1);

            b = GetLight(x,     y,     z, 2) +
                GetLight(x,     y + 1, z, 2) +
                GetLight(x + 1, y + 1, z, 2) +
                GetLight(x + 1, y,     z, 2);

            s = GetLight(x,     y,     z, 3) +
                GetLight(x,     y + 1, z, 3) +
                GetLight(x + 1, y + 1, z, 3) +
                GetLight(x + 1, y,     z, 3);
        } else {  // if vertex == 3
            r = GetLight(x,     y,     z, 0) +
                GetLight(x,     y - 1, z, 0) +
                GetLight(x + 1, y,     z, 0) +
                GetLight(x + 1, y - 1, z, 0);

            g = GetLight(x,     y,     z, 1) +
                GetLight(x,     y - 1, z, 1) +
                GetLight(x + 1, y,     z, 1) +
                GetLight(x + 1, y - 1, z, 1);

            b = GetLight(x,     y,     z, 2) +
                GetLight(x,     y - 1, z, 2) +
                GetLight(x + 1, y,     z, 2) +
                GetLight(x + 1, y - 1, z, 2);

            s = GetLight(x,     y,     z, 3) +
                GetLight(x,     y - 1, z, 3) +
                GetLight(x + 1, y,     z, 3) +
                GetLight(x + 1, y - 1, z, 3);
        }
    }

    return ((static_cast<uint32_t>(r) << 18) | (static_cast<uint32_t>(g) << 12) | (static_cast<uint32_t>(b) << 6) | s);
}

inline void Chunk::PushBack(Vertex vertex) {
    vertex_data[vertex_data_size] = vertex;
    ++vertex_data_size;

    if (vertex_data_size == vertex_data_capacity) {
        vertex_data_capacity *= 2;
        Vertex* new_vertex_data = new Vertex[vertex_data_capacity];

        if (new_vertex_data == nullptr) {
            std::cout << "Bad alloc: new_vertex_data" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        for (int i = 0; i < vertex_data_size; ++i) {
            new_vertex_data[i] = vertex_data[i];
        }
        delete[] vertex_data;

        vertex_data = new_vertex_data;
    }
}

// Good luck understanding this

void Chunk::GreedyMesh() {

    uint16_t is_processed[Chunk::DIRECTION_SIZE];

    uint16_t bit_mask;
    uint16_t next_bit_mask;

    uint16_t row_mask;
    uint16_t current_row;

    int w, h;

    int direction;
    int plane;
    int row;
    int bit;
    int next_bit;
    int next_row;

    uint8_t texture_id, next_texture_id;
    uint32_t l0, l1, l2, l3, next_l0, next_l1, next_l2, next_l3;

    bool flag;
    for (direction = 0; direction < Chunk::FACES_COUNT_PER_CUBE; ++direction) {
        for (plane = 0; plane < Chunk::DIRECTION_SIZE; ++plane) {

            std::fill(is_processed, is_processed + Chunk::DIRECTION_SIZE, 0);

            for (row = 0; row < Chunk::DIRECTION_SIZE; ++row) {
                current_row = face_planes_[direction][plane * Chunk::DIRECTION_SIZE + row];

                if (is_processed[row] == 0xFFFF || !current_row) {
                    continue;
                }

                for (bit = 0; bit < Chunk::DIRECTION_SIZE;) {
                    bit_mask = (1 << bit);

                    if ((is_processed[row] & bit_mask) || !(current_row & bit_mask)) {
                        ++bit;
                        continue;
                    }

                    if (direction == 0) {
                        l0 = Light(plane + 1, bit, row, 0, 0);
                        l1 = Light(plane + 1, bit, row, 0, 1);
                        l2 = Light(plane + 1, bit, row, 0, 2);
                        l3 = Light(plane + 1, bit, row, 0, 3);
                        texture_id = Blocks::blocks[voxels_[(bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id].texture_id[0];
                    } else if (direction == 1) {
                        l0 = Light(plane - 1, bit, row, 1, 0);
                        l1 = Light(plane - 1, bit, row, 1, 1);
                        l2 = Light(plane - 1, bit, row, 1, 2);
                        l3 = Light(plane - 1, bit, row, 1, 3);
                        texture_id = Blocks::blocks[voxels_[(bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id].texture_id[1];
                    } else if (direction == 2) {
                        l0 = Light(row, plane + 1, bit, 2, 0);
                        l1 = Light(row, plane + 1, bit, 2, 1);
                        l2 = Light(row, plane + 1, bit, 2, 2);
                        l3 = Light(row, plane + 1, bit, 2, 3);
                        texture_id = Blocks::blocks[voxels_[(plane * Chunk::DEPTH + bit) * Chunk::WIDTH + row].id].texture_id[2];
                    } else if (direction == 3) {
                        l0 = Light(row, plane - 1, bit, 3, 0);
                        l1 = Light(row, plane - 1, bit, 3, 1);
                        l2 = Light(row, plane - 1, bit, 3, 2);
                        l3 = Light(row, plane - 1, bit, 3, 3);
                        texture_id = Blocks::blocks[voxels_[(plane * Chunk::DEPTH + bit) * Chunk::WIDTH + row].id].texture_id[3];
                    } else if (direction == 4) {
                        l0 = Light(bit, row, plane + 1, 4, 0);
                        l1 = Light(bit, row, plane + 1, 4, 1);
                        l2 = Light(bit, row, plane + 1, 4, 2);
                        l3 = Light(bit, row, plane + 1, 4, 3);
                        texture_id = Blocks::blocks[voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + bit].id].texture_id[4];
                    } else {  // if (direction == 5)
                        l0 = Light(bit, row, plane - 1, 5, 0);
                        l1 = Light(bit, row, plane - 1, 5, 1);
                        l2 = Light(bit, row, plane - 1, 5, 2);
                        l3 = Light(bit, row, plane - 1, 5, 3);
                        texture_id = Blocks::blocks[voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + bit].id].texture_id[5];
                    }

                    if (l0 == 0 && l1 == 0 && l2 == 0 && l3 == 0) {
                        w = h = 1;
                        row_mask = bit_mask;
                        is_processed[row] |= bit_mask;


                        for (next_bit = bit + 1; next_bit < Chunk::DIRECTION_SIZE; ++next_bit) {
                            next_bit_mask = (1 << next_bit);

                            if ((is_processed[row] & next_bit_mask) || !(current_row & next_bit_mask)) {
                                break;
                            }

                            if (direction == 0) {
                                next_l0 = Light(plane + 1, next_bit, row, 0, 0);
                                next_l1 = Light(plane + 1, next_bit, row, 0, 1);
                                next_l2 = Light(plane + 1, next_bit, row, 0, 2);
                                next_l3 = Light(plane + 1, next_bit, row, 0, 3);
                            } else if (direction == 1) {
                                next_l0 = Light(plane - 1, next_bit, row, 1, 0);
                                next_l1 = Light(plane - 1, next_bit, row, 1, 1);
                                next_l2 = Light(plane - 1, next_bit, row, 1, 2);
                                next_l3 = Light(plane - 1, next_bit, row, 1, 3);
                            } else if (direction == 2) {
                                next_l0 = Light(row, plane + 1, next_bit, 2, 0);
                                next_l1 = Light(row, plane + 1, next_bit, 2, 1);
                                next_l2 = Light(row, plane + 1, next_bit, 2, 2);
                                next_l3 = Light(row, plane + 1, next_bit, 2, 3);
                            } else if (direction == 3) {
                                next_l0 = Light(row, plane - 1, next_bit, 3, 0);
                                next_l1 = Light(row, plane - 1, next_bit, 3, 1);
                                next_l2 = Light(row, plane - 1, next_bit, 3, 2);
                                next_l3 = Light(row, plane - 1, next_bit, 3, 3);
                            } else if (direction == 4) {
                                next_l0 = Light(next_bit, row, plane + 1, 4, 0);
                                next_l1 = Light(next_bit, row, plane + 1, 4, 1);
                                next_l2 = Light(next_bit, row, plane + 1, 4, 2);
                                next_l3 = Light(next_bit, row, plane + 1, 4, 3);
                            } else { // if (direction == 5)
                                next_l0 = Light(next_bit, row, plane - 1, 5, 0);
                                next_l1 = Light(next_bit, row, plane - 1, 5, 1);
                                next_l2 = Light(next_bit, row, plane - 1, 5, 2);
                                next_l3 = Light(next_bit, row, plane - 1, 5, 3);
                            }

                            if (l0 != next_l0 || l1 != next_l1 || l2 != next_l2 || l3 != next_l3) {

                                if ((next_l0 != next_l1 || next_l2 != next_l3) && (next_l0 != next_l3 || next_l1 != next_l2)) {
                                    is_processed[row] |= next_bit_mask;

                                    if (direction == 0) {
                                        next_texture_id =
                                            Blocks::blocks[voxels_[(next_bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id].texture_id[0];
                                    } else if (direction == 1) {
                                        next_texture_id =
                                            Blocks::blocks[voxels_[(next_bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id].texture_id[1];
                                    } else if (direction == 2) {
                                        next_texture_id =
                                            Blocks::blocks[voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + row].id].texture_id[2];
                                    } else if (direction == 3) {
                                        next_texture_id =
                                            Blocks::blocks[voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + row].id].texture_id[3];
                                    } else if (direction == 4) {
                                        next_texture_id =
                                            Blocks::blocks[voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id].texture_id[4];
                                    } else { // if (direction == 5)
                                        next_texture_id =
                                            Blocks::blocks[voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id].texture_id[5];
                                    }

                                    ADD_FACE(next_l0, next_l1, next_l2, next_l3, next_texture_id, plane, row, next_bit, 1, 1);
                                }

                                break;
                            }

                            ++w;
                            row_mask |= next_bit_mask;
                            is_processed[row] |= next_bit_mask;
                        }


                        flag = false;
                        for (next_row = row + 1; next_row < Chunk::DIRECTION_SIZE; ++next_row) {

                            if ((face_planes_[direction][plane * Chunk::DIRECTION_SIZE + next_row] & row_mask) != row_mask) {
                                break;
                            }

                            for (next_bit = bit; next_bit < bit + w; ++next_bit) {

                                if (direction == 0) {
                                    next_l0 = Light(plane + 1, next_bit, next_row, 0, 0);
                                    next_l1 = Light(plane + 1, next_bit, next_row, 0, 1);
                                    next_l2 = Light(plane + 1, next_bit, next_row, 0, 2);
                                    next_l3 = Light(plane + 1, next_bit, next_row, 0, 3);
                                } else if (direction == 1) {
                                    next_l0 = Light(plane - 1, next_bit, next_row, 1, 0);
                                    next_l1 = Light(plane - 1, next_bit, next_row, 1, 1);
                                    next_l2 = Light(plane - 1, next_bit, next_row, 1, 2);
                                    next_l3 = Light(plane - 1, next_bit, next_row, 1, 3);
                                } else if (direction == 2) {
                                    next_l0 = Light(next_row, plane + 1, next_bit, 2, 0);
                                    next_l1 = Light(next_row, plane + 1, next_bit, 2, 1);
                                    next_l2 = Light(next_row, plane + 1, next_bit, 2, 2);
                                    next_l3 = Light(next_row, plane + 1, next_bit, 2, 3);
                                } else if (direction == 3) {
                                    next_l0 = Light(next_row, plane - 1, next_bit, 3, 0);
                                    next_l1 = Light(next_row, plane - 1, next_bit, 3, 1);
                                    next_l2 = Light(next_row, plane - 1, next_bit, 3, 2);
                                    next_l3 = Light(next_row, plane - 1, next_bit, 3, 3);
                                } else if (direction == 4) {
                                    next_l0 = Light(next_bit, next_row, plane + 1, 4, 0);
                                    next_l1 = Light(next_bit, next_row, plane + 1, 4, 1);
                                    next_l2 = Light(next_bit, next_row, plane + 1, 4, 2);
                                    next_l3 = Light(next_bit, next_row, plane + 1, 4, 3);
                                } else { // if (direction == 5)
                                    next_l0 = Light(next_bit, next_row, plane - 1, 5, 0);
                                    next_l1 = Light(next_bit, next_row, plane - 1, 5, 1);
                                    next_l2 = Light(next_bit, next_row, plane - 1, 5, 2);
                                    next_l3 = Light(next_bit, next_row, plane - 1, 5, 3);
                                }

                                if (l0 != next_l0 || l1 != next_l1 || l2 != next_l2 || l3 != next_l3) {
                                    flag = true;

                                    if ((next_l0 != next_l1 || next_l2 != next_l3) && (next_l0 != next_l3 || next_l1 != next_l2)) {
                                        is_processed[next_row] |= (1 << next_bit);

                                        if (direction == 0) {
                                            next_texture_id =
                                                Blocks::blocks[voxels_[(next_bit * Chunk::DEPTH + next_row) * Chunk::WIDTH + plane].id].texture_id[0];
                                        } else if (direction == 1) {
                                            next_texture_id =
                                                Blocks::blocks[voxels_[(next_bit * Chunk::DEPTH + next_row) * Chunk::WIDTH + plane].id].texture_id[1];
                                        } else if (direction == 2) {
                                            next_texture_id =
                                                Blocks::blocks[voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + next_row].id].texture_id[2];
                                        } else if (direction == 3) {
                                            next_texture_id =
                                                Blocks::blocks[voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + next_row].id].texture_id[3];
                                        } else if (direction == 4) {
                                            next_texture_id =
                                                Blocks::blocks[voxels_[(next_row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id].texture_id[4];
                                        } else { // if (direction == 5)
                                            next_texture_id =
                                                Blocks::blocks[voxels_[(next_row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id].texture_id[5];
                                        }

                                        ADD_FACE(next_l0, next_l1, next_l2, next_l3, next_texture_id, plane, next_row, next_bit, 1, 1);
                                    }

                                    break;
                                }
                            }
                            if (flag) {
                                break;
                            }

                            ++h;
                            is_processed[next_row] |= row_mask;
                        }

                        ADD_FACE(l0, l1, l2, l3, texture_id, plane, row, bit, w, h);

                        bit += w;
                        continue;
                    } else if ((l0 != l1 || l2 != l3) && (l0 != l3 || l1 != l2)) {
                        is_processed[row] |= bit_mask;

                        ADD_FACE(l0, l1, l2, l3, texture_id, plane, row, bit, 1, 1);

                        ++bit;
                        continue;
                    }

                    w = h = 1;
                    row_mask = bit_mask;
                    is_processed[row] |= bit_mask;


                    for (next_bit = bit + 1; next_bit < Chunk::DIRECTION_SIZE; ++next_bit) {
                        next_bit_mask = (1 << next_bit);

                        if ((is_processed[row] & next_bit_mask) || !(current_row & next_bit_mask)) {
                            break;
                        }

                        if (direction == 0) {
                            next_l0 = Light(plane + 1, next_bit, row, 0, 0);
                            next_l1 = Light(plane + 1, next_bit, row, 0, 1);
                            next_l2 = Light(plane + 1, next_bit, row, 0, 2);
                            next_l3 = Light(plane + 1, next_bit, row, 0, 3);
                            next_texture_id = Blocks::blocks[voxels_[(next_bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id].texture_id[0];
                        } else if (direction == 1) {
                            next_l0 = Light(plane - 1, next_bit, row, 1, 0);
                            next_l1 = Light(plane - 1, next_bit, row, 1, 1);
                            next_l2 = Light(plane - 1, next_bit, row, 1, 2);
                            next_l3 = Light(plane - 1, next_bit, row, 1, 3);
                            next_texture_id = Blocks::blocks[voxels_[(next_bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id].texture_id[1];
                        } else if (direction == 2) {
                            next_l0 = Light(row, plane + 1, next_bit, 2, 0);
                            next_l1 = Light(row, plane + 1, next_bit, 2, 1);
                            next_l2 = Light(row, plane + 1, next_bit, 2, 2);
                            next_l3 = Light(row, plane + 1, next_bit, 2, 3);
                            next_texture_id = Blocks::blocks[voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + row].id].texture_id[2];
                        } else if (direction == 3) {
                            next_l0 = Light(row, plane - 1, next_bit, 3, 0);
                            next_l1 = Light(row, plane - 1, next_bit, 3, 1);
                            next_l2 = Light(row, plane - 1, next_bit, 3, 2);
                            next_l3 = Light(row, plane - 1, next_bit, 3, 3);
                            next_texture_id = Blocks::blocks[voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + row].id].texture_id[3];
                        } else if (direction == 4) {
                            next_l0 = Light(next_bit, row, plane + 1, 4, 0);
                            next_l1 = Light(next_bit, row, plane + 1, 4, 1);
                            next_l2 = Light(next_bit, row, plane + 1, 4, 2);
                            next_l3 = Light(next_bit, row, plane + 1, 4, 3);
                            next_texture_id = Blocks::blocks[voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id].texture_id[4];
                        } else { // if (direction == 5)
                            next_l0 = Light(next_bit, row, plane - 1, 5, 0);
                            next_l1 = Light(next_bit, row, plane - 1, 5, 1);
                            next_l2 = Light(next_bit, row, plane - 1, 5, 2);
                            next_l3 = Light(next_bit, row, plane - 1, 5, 3);
                            next_texture_id = Blocks::blocks[voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id].texture_id[5];
                        }

                        if (l0 != next_l0 || l1 != next_l1 || l2 != next_l2 || l3 != next_l3 || texture_id != next_texture_id) {

                            if ((next_l0 != next_l1 || next_l2 != next_l3) && (next_l0 != next_l3 || next_l1 != next_l2)) {
                                is_processed[row] |= next_bit_mask;

                                ADD_FACE(next_l0, next_l1, next_l2, next_l3, next_texture_id, plane, row, next_bit, 1, 1);
                            }

                            break;
                        }

                        ++w;
                        row_mask |= next_bit_mask;
                        is_processed[row] |= next_bit_mask;
                    }


                    flag = false;
                    for (next_row = row + 1; next_row < Chunk::DIRECTION_SIZE; ++next_row) {

                        if ((face_planes_[direction][plane * Chunk::DIRECTION_SIZE + next_row] & row_mask) != row_mask) {
                            break;
                        }

                        for (next_bit = bit; next_bit < bit + w; ++next_bit) {

                            if (direction == 0) {
                                next_l0 = Light(plane + 1, next_bit, next_row, 0, 0);
                                next_l1 = Light(plane + 1, next_bit, next_row, 0, 1);
                                next_l2 = Light(plane + 1, next_bit, next_row, 0, 2);
                                next_l3 = Light(plane + 1, next_bit, next_row, 0, 3);
                                next_texture_id = Blocks::blocks[voxels_[(next_bit * Chunk::DEPTH + next_row) * Chunk::WIDTH + plane].id].texture_id[0];
                            } else if (direction == 1) {
                                next_l0 = Light(plane - 1, next_bit, next_row, 1, 0);
                                next_l1 = Light(plane - 1, next_bit, next_row, 1, 1);
                                next_l2 = Light(plane - 1, next_bit, next_row, 1, 2);
                                next_l3 = Light(plane - 1, next_bit, next_row, 1, 3);
                                next_texture_id = Blocks::blocks[voxels_[(next_bit * Chunk::DEPTH + next_row) * Chunk::WIDTH + plane].id].texture_id[1];
                            } else if (direction == 2) {
                                next_l0 = Light(next_row, plane + 1, next_bit, 2, 0);
                                next_l1 = Light(next_row, plane + 1, next_bit, 2, 1);
                                next_l2 = Light(next_row, plane + 1, next_bit, 2, 2);
                                next_l3 = Light(next_row, plane + 1, next_bit, 2, 3);
                                next_texture_id = Blocks::blocks[voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + next_row].id].texture_id[2];
                            } else if (direction == 3) {
                                next_l0 = Light(next_row, plane - 1, next_bit, 3, 0);
                                next_l1 = Light(next_row, plane - 1, next_bit, 3, 1);
                                next_l2 = Light(next_row, plane - 1, next_bit, 3, 2);
                                next_l3 = Light(next_row, plane - 1, next_bit, 3, 3);
                                next_texture_id = Blocks::blocks[voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + next_row].id].texture_id[3];
                            } else if (direction == 4) {
                                next_l0 = Light(next_bit, next_row, plane + 1, 4, 0);
                                next_l1 = Light(next_bit, next_row, plane + 1, 4, 1);
                                next_l2 = Light(next_bit, next_row, plane + 1, 4, 2);
                                next_l3 = Light(next_bit, next_row, plane + 1, 4, 3);
                                next_texture_id = Blocks::blocks[voxels_[(next_row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id].texture_id[4];
                            } else { // if (direction == 5)
                                next_l0 = Light(next_bit, next_row, plane - 1, 5, 0);
                                next_l1 = Light(next_bit, next_row, plane - 1, 5, 1);
                                next_l2 = Light(next_bit, next_row, plane - 1, 5, 2);
                                next_l3 = Light(next_bit, next_row, plane - 1, 5, 3);
                                next_texture_id = Blocks::blocks[voxels_[(next_row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id].texture_id[5];
                            }

                            if (l0 != next_l0 || l1 != next_l1 || l2 != next_l2 || l3 != next_l3 || texture_id != next_texture_id) {
                                flag = true;

                                if ((next_l0 != next_l1 || next_l2 != next_l3) && (next_l0 != next_l3 || next_l1 != next_l2)) {
                                    is_processed[next_row] |= (1 << next_bit);

                                    ADD_FACE(next_l0, next_l1, next_l2, next_l3, next_texture_id, plane, next_row, next_bit, 1, 1);
                                }


                                break;
                            }
                        }
                        if (flag) {
                            break;
                        }

                        ++h;
                        is_processed[next_row] |= row_mask;
                    }


                    ADD_FACE(l0, l1, l2, l3, texture_id, plane, row, bit, w, h);

                    bit += w;
                }
            }
        }
    }
}

Chunk::~Chunk() {
    delete[] lightmap_;
    delete[] vertex_data;
}
