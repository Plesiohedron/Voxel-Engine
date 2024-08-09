#include "Chunk.h"
#include "Chunks.h"

#include <immintrin.h>

Chunk** Chunk::chunk_storage_;

Chunk::Chunk(const glm::ivec3& coordinates)
    : global_coordinates{coordinates.x - Chunks::storage_sizes.x / 2, coordinates.y, coordinates.z - Chunks::storage_sizes.z / 2},
      local_coordinates{coordinates.x, coordinates.y, coordinates.z}, is_modified{true} {

    face_planes_[0] = new uint16_t[DIRECTION_SIZE_P2 * FACES_COUNT_PER_CUBE]{};

    for (int i = 0; i < Chunk::FACES_COUNT_PER_CUBE; ++i) {
        if (i < 2) {
            face_planes_[i] = &face_planes_[0][DIRECTION_SIZE_P2 * i];
        } else if (i < 4) {
            face_planes_[i] = &face_planes_[0][DIRECTION_SIZE_P2 * i];
        } else {
            face_planes_[i] = &face_planes_[0][DIRECTION_SIZE_P2 * i];
        }
    }

    voxels_ = new Voxel[Chunk::VOLUME];
    vertex_data = new uint64_t[vertex_data_capacity];

    if (voxels_ == nullptr) {
        std::cout << "Bad alloc: voxels_" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    if (vertex_data == nullptr) {
        std::cout << "Bad alloc: vertex_data" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    alignas(32) uint16_t X_rows[Chunk::HEIGHT][Chunk::DEPTH]{};
    alignas(32) uint16_t Y_rows[Chunk::DEPTH][Chunk::WIDTH]{};
    alignas(32) uint16_t Z_rows[Chunk::WIDTH][Chunk::HEIGHT]{};

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                int global_x = x + global_coordinates.x * Chunk::WIDTH;
                int global_y = y + global_coordinates.y * Chunk::DEPTH;
                int global_z = z + global_coordinates.z * Chunk::HEIGHT;

                //uint8_t id = global_y <= std::sin(0.1 * global_x) * 10;
                uint8_t id = global_y <= std::sin(0.1 * global_x) * 10 + std::cos(0.1 * global_z) * 10;
                //uint8_t id = global_y <= 10;
                if (global_y <= 2) {
                    id = 4;
                }
                /*if (global_x > 2) {
                    id = 0;
                }
                if (global_z > 3) {
                    id = 0;
                }*/

                //uint8_t id = 0 + (std::rand() % 2);
                //uint8_t id = 0;

                //if ((global_x + global_y + global_z) % 2) {
                //    id = 1;
                //}

                if (id) {
                    X_rows[y][z] |= (1 << x);
                    Y_rows[z][x] |= (1 << y);
                    Z_rows[x][y] |= (1 << z);
                }

                // voxels[y][z][x] := voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]
                voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id = id;
            }
        }
    }

    Culling(X_rows, Y_rows, Z_rows);
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

    if (local_coordinates.x == 0) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            row_mask2 = 0;
            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                row_mask2 |= (IsBlocked(16, y, z) << y);
            }

            face_planes_[0][15 * Chunk::DEPTH + z] &= ~row_mask2;
        }
    } else if (local_coordinates.x == Chunks::storage_sizes.x - 1) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            row_mask1 = 0;
            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                row_mask1 |= (IsBlocked(-1, y, z) << y);
            }

            face_planes_[1][0 * Chunk::DEPTH + z] &= ~row_mask1;
        }
    } else {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            row_mask1 = 0;
            row_mask2 = 0;
            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                row_mask1 |= (IsBlocked(-1, y, z) << y);
                row_mask2 |= (IsBlocked(16, y, z) << y);
            }

            face_planes_[0][15 * Chunk::DEPTH + z] &= ~row_mask2;
            face_planes_[1][0 * Chunk::DEPTH + z] &= ~row_mask1;
        }
    }

    if (local_coordinates.y == 0) {
        for (int x = 0; x < Chunk::WIDTH; ++x) {
            row_mask2 = 0;
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                row_mask2 |= (IsBlocked(x, 16, z) << z);
            }

            face_planes_[2][15 * Chunk::WIDTH + x] &= ~row_mask2;
        }
    } else if (local_coordinates.y == Chunks::storage_sizes.y - 1) {
        for (int x = 0; x < Chunk::WIDTH; ++x) {
            row_mask1 = 0;
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                row_mask1 |= (IsBlocked(x, -1, z) << z);
            }

            face_planes_[3][0 * Chunk::WIDTH + x] &= ~row_mask1;
        }
    } else {
        for (int x = 0; x < Chunk::WIDTH; ++x) {
            row_mask1 = 0;
            row_mask2 = 0;
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                row_mask1 |= (IsBlocked(x, -1, z) << z);
                row_mask2 |= (IsBlocked(x, 16, z) << z);
            }

            face_planes_[2][15 * Chunk::WIDTH + x] &= ~row_mask2;
            face_planes_[3][0 * Chunk::WIDTH + x] &= ~row_mask1;
        }
    }

    if (local_coordinates.z == 0) {
        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            row_mask2 = 0;
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                row_mask2 |= (IsBlocked(x, y, 16) << x);
            }

            face_planes_[4][15 * Chunk::HEIGHT + y] &= ~row_mask2;
        }
    } else if (local_coordinates.z == Chunks::storage_sizes.z - 1) {
        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            row_mask1 = 0;
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                row_mask1 |= (IsBlocked(x, y, -1) << x);
            }

            face_planes_[5][0 * Chunk::HEIGHT + y] &= ~row_mask1;
        }
    } else {
        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            row_mask1 = 0;
            row_mask2 = 0;
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                row_mask1 |= (IsBlocked(x, y, -1) << x);
                row_mask2 |= (IsBlocked(x, y, 16) << x);
            }

            face_planes_[4][15 * Chunk::HEIGHT + y] &= ~row_mask2;
            face_planes_[5][0 * Chunk::HEIGHT + y] &= ~row_mask1;
        }
    }
}

bool Chunk::IsBlocked(int x, int y, int z) {
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

        if (0 <= X && X < Chunks::storage_sizes.x && 0 <= Y && Y < Chunks::storage_sizes.y && 0 <= Z && Z < Chunks::storage_sizes.z) {
            return chunk_storage_[(Y * Chunks::storage_sizes.z + Z) * Chunks::storage_sizes.x + X]
                   ->voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id;
        }

    }

    return false;
}

uint64_t Chunk::AmbientOcclusion(int x, int y, int z, int direction) {
    uint32_t side1[VERTICES_COUNT_PER_SQUARE]{};
    uint32_t side2[VERTICES_COUNT_PER_SQUARE]{};
    uint32_t corner[VERTICES_COUNT_PER_SQUARE]{};

    if (direction == 0) {
        corner[0] = IsBlocked(x + 1, y + 1, z + 1);
        side2[0] = side1[1] = IsBlocked(x + 1, y + 1, z);
        corner[1] = IsBlocked(x + 1, y + 1, z - 1);
        side2[1] = side1[2] = IsBlocked(x + 1, y, z - 1);
        corner[2] = IsBlocked(x + 1, y - 1, z - 1);
        side2[2] = side1[3] = IsBlocked(x + 1, y - 1, z);
        corner[3] = IsBlocked(x + 1, y - 1, z + 1);
        side2[3] = side1[0] = IsBlocked(x + 1, y, z + 1);
    } else if (direction == 1) {
        corner[0] = IsBlocked(x - 1, y + 1, z - 1);
        side2[0] = side1[1] = IsBlocked(x - 1, y + 1, z);
        corner[1] = IsBlocked(x - 1, y + 1, z + 1);
        side2[1] = side1[2] = IsBlocked(x - 1, y, z + 1);
        corner[2] = IsBlocked(x - 1, y - 1, z + 1);
        side2[2] = side1[3] = IsBlocked(x - 1, y - 1, z);
        corner[3] = IsBlocked(x - 1, y - 1, z - 1);
        side2[3] = side1[0] = IsBlocked(x - 1, y, z - 1);
    } else if (direction == 2) {
        corner[0] = IsBlocked(x + 1, y + 1, z + 1);
        side2[0] = side1[1] = IsBlocked(x, y + 1, z + 1);
        corner[1] = IsBlocked(x - 1, y + 1, z + 1);
        side2[1] = side1[2] = IsBlocked(x - 1, y + 1, z);
        corner[2] = IsBlocked(x - 1, y + 1, z - 1);
        side2[2] = side1[3] = IsBlocked(x, y + 1, z - 1);
        corner[3] = IsBlocked(x + 1, y + 1, z - 1);
        side2[3] = side1[0] = IsBlocked(x + 1, y + 1, z);
    } else if (direction == 3) {
        corner[0] = IsBlocked(x - 1, y - 1, z + 1);
        side2[0] = side1[1] = IsBlocked(x, y - 1, z + 1);
        corner[1] = IsBlocked(x + 1, y - 1, z + 1);
        side2[1] = side1[2] = IsBlocked(x + 1, y - 1, z);
        corner[2] = IsBlocked(x + 1, y - 1, z - 1);
        side2[2] = side1[3] = IsBlocked(x, y - 1, z - 1);
        corner[3] = IsBlocked(x - 1, y - 1, z - 1);
        side2[3] = side1[0] = IsBlocked(x - 1, y - 1, z);
    } else if (direction == 4) {
        corner[0] = IsBlocked(x + 1, y - 1, z + 1);
        side2[0] = side1[1] = IsBlocked(x + 1, y, z + 1);
        corner[1] = IsBlocked(x + 1, y + 1, z + 1);
        side2[1] = side1[2] = IsBlocked(x, y + 1, z + 1);
        corner[2] = IsBlocked(x - 1, y + 1, z + 1);
        side2[2] = side1[3] = IsBlocked(x - 1, y, z + 1);
        corner[3] = IsBlocked(x - 1, y - 1, z + 1);
        side2[3] = side1[0] = IsBlocked(x, y - 1, z + 1);
    } else { // if (direction == 5)
        corner[0] = IsBlocked(x + 1, y + 1, z - 1);
        side2[0] = side1[1] = IsBlocked(x + 1, y, z - 1);
        corner[1] = IsBlocked(x + 1, y - 1, z - 1);
        side2[1] = side1[2] = IsBlocked(x, y - 1, z - 1);
        corner[2] = IsBlocked(x - 1, y - 1, z - 1);
        side2[2] = side1[3] = IsBlocked(x - 1, y, z - 1);
        corner[3] = IsBlocked(x - 1, y + 1, z - 1);
        side2[3] = side1[0] = IsBlocked(x, y + 1, z - 1);
    }

    uint64_t result = 0;
    for (int i = 0; i < Chunk::VERTICES_COUNT_PER_SQUARE; ++i) {
        result |= ((side2[i] + corner[i] + side1[i]) << (i * 2));
        result |= ((side2[i] << (10 + i * 3)) | (corner[i] << (9 + i * 3)) | (side1[i] << (8 + i * 3)));
    }

    return result;
}

inline void Chunk::PushBack(uint64_t vertex) {
    vertex_data[vertex_data_size] = vertex;
    ++vertex_data_size;

    if (vertex_data_size == vertex_data_capacity) {
        vertex_data_capacity *= 2;
        uint64_t* new_vertex_data = new uint64_t[vertex_data_capacity];

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

    uint64_t current_type;
    uint64_t next_type;

    uint32_t brithness;
    uint32_t w, h;
    uint32_t vertexAO[Chunk::VERTICES_COUNT_PER_SQUARE];

    uint32_t direction;
    uint32_t plane;
    uint32_t row;
    uint32_t bit;
    uint32_t next_bit;
    uint32_t next_row;

    bool flag;
    for (direction = 0; direction < Chunk::FACES_COUNT_PER_CUBE; ++direction) {
        if (direction < 2) {
            brithness = 14;
        } else if (direction == 2) {
            brithness = 15;
        } else if (direction == 3) {
            brithness = 12;
        } else { // if (direction < 6)
            brithness = 13;
        }

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

                    if (direction < 2) {
                        current_type = ((AmbientOcclusion(plane, bit, row, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                        (brithness << 12) | (15u << 8) | voxels_[(bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id);
                    } else if (direction < 4) {
                        current_type = ((AmbientOcclusion(row, plane, bit, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                        (brithness << 12) | (15u << 8) | voxels_[(plane * Chunk::DEPTH + bit) * Chunk::WIDTH + row].id);
                    } else { // if (direction < 6)
                        current_type = ((AmbientOcclusion(bit, row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                        (brithness << 12) | (15u << 8) | voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + bit].id);
                    }
                    w = h = 1;
                    row_mask = bit_mask;
                    is_processed[row] |= bit_mask;


                    for (next_bit = bit + 1; next_bit < Chunk::DIRECTION_SIZE; ++next_bit) {
                        next_bit_mask = (1 << next_bit);

                        if ((is_processed[row] & next_bit_mask) || !(current_row & next_bit_mask)) {
                            break;
                        }

                        if (direction < 2) {
                            next_type = ((AmbientOcclusion(plane, next_bit, row, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                         (brithness << 12) | (15u << 8) | voxels_[(next_bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id);
                        } else if (direction < 4) {
                            next_type = ((AmbientOcclusion(row, plane, next_bit, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                         (brithness << 12) | (15u << 8) | voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + row].id);
                        } else { // if (direction < 6)
                            next_type = ((AmbientOcclusion(next_bit, row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                         (brithness << 12) | (15u << 8) | voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id);
                        }

                        if (current_type != next_type) {
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

                            if (direction < 2) {
                                next_type =
                                    ((AmbientOcclusion(plane, next_bit, next_row, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                     (brithness << 12) | (15u << 8) | voxels_[(next_bit * Chunk::DEPTH + next_row) * Chunk::WIDTH + plane].id);
                            } else if (direction < 4) {
                                next_type =
                                    ((AmbientOcclusion(next_row, plane, next_bit, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                     (brithness << 12) | (15u << 8) | voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + next_row].id);
                            } else { // if (direction < 6)
                                next_type =
                                    ((AmbientOcclusion(next_bit, next_row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                     (brithness << 12) | (15u << 8) | voxels_[(next_row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id);
                            }

                            if (current_type != next_type) {
                                flag = true;
                                break;
                            }
                        }
                        if (flag) {
                            break;
                        }

                        ++h;
                        is_processed[next_row] |= row_mask;
                    }

                    vertexAO[0] = (current_type >> 24) & 3;
                    vertexAO[1] = (current_type >> 26) & 3;
                    vertexAO[2] = (current_type >> 28) & 3;
                    vertexAO[3] = (current_type >> 30) & 3;

                    if (vertexAO[0] + vertexAO[2] < vertexAO[1] + vertexAO[3]) {
                        
                        if (direction == 0) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     ((plane + 1) << 24) | (bit << 18)       | (row << 12)       | (h << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     ((plane + 1) << 24) | ((bit + w) << 18) | (row << 12)       | (h << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     ((plane + 1) << 24) | ((bit + w) << 18) | ((row + h) << 12) | (0 << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     ((plane + 1) << 24) | (bit << 18)       | ((row + h) << 12) | (0 << 6) | w);
                        } else if (direction == 1) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     (plane << 24) | (bit << 18)       | ((row + h) << 12) | (h << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     (plane << 24) | ((bit + w) << 18) | ((row + h) << 12) | (h << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     (plane << 24) | ((bit + w) << 18) | (row << 12)       | (0 << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     (plane << 24) | (bit << 18)       | (row << 12)       | (0 << 6) | w);
                        } else if (direction == 2) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     (row << 24)       | ((plane + 1) << 18) | (bit << 12)       | (0 << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     (row << 24)       | ((plane + 1) << 18) | ((bit + w) << 12) | (0 << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     ((row + h) << 24) | ((plane + 1) << 18) | ((bit + w) << 12) | (h << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     ((row + h) << 24) | ((plane + 1) << 18) | (bit << 12)       | (h << 6) | 0);
                        } else if (direction == 3) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     ((row + h) << 24) | (plane << 18) | (bit << 12)       | (h << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     ((row + h) << 24) | (plane << 18) | ((bit + w) << 12) | (h << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     (row << 24)       | (plane << 18) | ((bit + w) << 12) | (0 << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     (row << 24)       | (plane << 18) | (bit << 12)       | (0 << 6) | w);
                        } else if (direction == 4) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     (bit << 24)       | ((row + h) << 18) | ((plane + 1) << 12) | (0 << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     (bit << 24)       | (row << 18)       | ((plane + 1) << 12) | (0 << 6) | h);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     ((bit + w) << 24) | (row << 18)       | ((plane + 1) << 12) | (w << 6) | h);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     ((bit + w) << 24) | ((row + h) << 18) | ((plane + 1) << 12) | (w << 6) | 0);
                        } else { // if (direction == 5)
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     (bit << 24)       | (row << 18)       | (plane << 12) | (w << 6) | h);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     (bit << 24)       | ((row + h) << 18) | (plane << 12) | (w << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     ((bit + w) << 24) | ((row + h) << 18) | (plane << 12) | (0 << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     ((bit + w) << 24) | (row << 18)       | (plane << 12) | (0 << 6) | h);
                        }
                        
                    } else {
                        
                        if (direction == 0) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     ((plane + 1) << 24) | (bit << 18)       | ((row + h) << 12) | (0 << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     ((plane + 1) << 24) | (bit << 18)       | (row << 12)       | (h << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     ((plane + 1) << 24) | ((bit + w) << 18) | (row << 12)       | (h << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     ((plane + 1) << 24) | ((bit + w) << 18) | ((row + h) << 12) | (0 << 6) | 0);
                        } else if (direction == 1) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     (plane << 24) | (bit << 18)       | (row << 12)       | (0 << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     (plane << 24) | (bit << 18)       | ((row + h) << 12) | (h << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     (plane << 24) | ((bit + w) << 18) | ((row + h) << 12) | (h << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     (plane << 24) | ((bit + w) << 18) | (row << 12)       | (0 << 6) | 0);
                        } else if (direction == 2) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     ((row + h) << 24) | ((plane + 1) << 18) | (bit << 12)       | (h << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     (row << 24)       | ((plane + 1) << 18) | (bit << 12)       | (0 << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     (row << 24)       | ((plane + 1) << 18) | ((bit + w) << 12) | (0 << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     ((row + h) << 24) | ((plane + 1) << 18) | ((bit + w) << 12) | (h << 6) | w);
                        } else if (direction == 3) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     (row << 24)       | (plane << 18) | (bit << 12)       | (0 << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     ((row + h) << 24) | (plane << 18) | (bit << 12)       | (h << 6) | w);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     ((row + h) << 24) | (plane << 18) | ((bit + w) << 12) | (h << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     (row << 24)       | (plane << 18) | ((bit + w) << 12) | (0 << 6) | 0);
                        } else if (direction == 4) {
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     ((bit + w) << 24) | ((row + h) << 18) | ((plane + 1) << 12) | (w << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     (bit << 24)       | ((row + h) << 18) | ((plane + 1) << 12) | (0 << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     (bit << 24)       | (row << 18)       | ((plane + 1) << 12) | (0 << 6) | h);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     ((bit + w) << 24) | (row << 18)       | ((plane + 1) << 12) | (w << 6) | h);
                        } else { // if (direction == 5)
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[1] << 30) |
                                     ((bit + w) << 24) | (row << 18)       | (plane << 12) | (0 << 6) | h);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[2] << 30) |
                                     (bit << 24)       | (row << 18)       | (plane << 12) | (w << 6) | h);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[3] << 30) |
                                     (bit << 24)       | ((row + h) << 18) | (plane << 12) | (w << 6) | 0);
                            PushBack(((current_type & 0x0000000000FFFFFF) << 32) | (vertexAO[0] << 30) |
                                     ((bit + w) << 24) | ((row + h) << 18) | (plane << 12) | (0 << 6) | 0);
                        }
                        
                    }

                    bit += w;
                }
            }
        }
    }
}

Chunk::~Chunk() {
    delete[] vertex_data;
    delete[] face_planes_[0];
    delete[] voxels_;
}