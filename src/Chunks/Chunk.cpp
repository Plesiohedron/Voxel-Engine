#include "Chunk.h"
#include "Chunks.h"

#include <immintrin.h>

Chunk** Chunk::chunk_storage_;

Chunk::Chunk(const glm::ivec3& coordinates)
    : global_coordinates{coordinates.x - Chunks::storage_sizes.x / 2, coordinates.y, coordinates.z - Chunks::storage_sizes.z / 2},
      local_coordinates{coordinates.x, coordinates.y, coordinates.z} {

    face_planes_[0] = new uint16_t[2 * (Chunk::WIDTH * Chunk::DEPTH + Chunk::HEIGHT * Chunk::WIDTH + Chunk::DEPTH * Chunk::HEIGHT)]{};

    for (int i = 0; i < Chunk::FACES_COUNT_PER_CUBE; ++i) {
        if (i < 2) {
            face_planes_[i] = &face_planes_[0][Chunk::WIDTH * Chunk::DEPTH * i];
        } else if (i < 4) {
            face_planes_[i] = &face_planes_[0][Chunk::HEIGHT * Chunk::WIDTH * i];
        } else {
            face_planes_[i] = &face_planes_[0][Chunk::DEPTH * Chunk::HEIGHT * i];
        }
    }

    voxels_ = new Voxel[Chunk::VOLUME];
    vertex_data.reserve(Chunk::VOXEL_FACES_CAPACITY * Chunk::VERTICES_COUNT_PER_SQUARE);
    index_data.reserve(Chunk::VOXEL_FACES_CAPACITY * Chunk::INDEXES_COUNT_PER_SQUARE);

    uint16_t X_rows[Chunk::HEIGHT][Chunk::DEPTH]{};
    uint16_t Y_rows[Chunk::DEPTH][Chunk::WIDTH]{};
    uint16_t Z_rows[Chunk::WIDTH][Chunk::HEIGHT]{};

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                int global_x = x + global_coordinates.x * Chunk::WIDTH;
                int global_y = y + global_coordinates.y * Chunk::DEPTH;
                int global_z = z + global_coordinates.z * Chunk::HEIGHT;

                //uint8_t id = global_y <= std::sin(0.1 * global_x) * 10;
                //uint8_t id = global_y <= std::sin(0.1 * global_x) * 10 + std::cos(0.1 * global_z) * 10;
                //uint8_t id = global_y <= 10;
                /*if (global_y <= 2) {
                    id = 4;
                }*/
                //uint8_t id = 0 + (std::rand() % 2);
                uint8_t id = (global_x + global_y + global_z) % 2;

                if (id) {
                    X_rows[y][z] |= (static_cast<uint16_t>(1) << x);
                    Y_rows[z][x] |= (static_cast<uint16_t>(1) << y);
                    Z_rows[x][y] |= (static_cast<uint16_t>(1) << z);
                }

                // voxels[y][z][x] := voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]
                voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id = id;
            }
        }
    }

    Culling(X_rows, Y_rows, Z_rows);
}

void Chunk::Culling(uint16_t (&X_rows)[Chunk::HEIGHT][Chunk::DEPTH], uint16_t (&Y_rows)[Chunk::DEPTH][Chunk::WIDTH],
                    uint16_t (&Z_rows)[Chunk::WIDTH][Chunk::HEIGHT]) {

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        __m256i X_plane = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&X_rows[y][0]));

        __m256i forward_faces = _mm256_and_si256(X_plane, _mm256_andnot_si256(_mm256_srli_epi16(X_plane, 1), _mm256_set1_epi16(0xFFFF)));
        __m256i back_faces = _mm256_and_si256(X_plane, _mm256_andnot_si256(_mm256_slli_epi16(X_plane, 1), _mm256_set1_epi16(0xFFFF)));

        uint16_t forward_faces_plane[16], back_faces_plane[16];
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(forward_faces_plane), forward_faces);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(back_faces_plane), back_faces);

        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                face_planes_[0][x * Chunk::DEPTH + z] |= ((forward_faces_plane[z] >> x) & static_cast<uint16_t>(1)) << y;
                face_planes_[1][x * Chunk::DEPTH + z] |= ((back_faces_plane[z] >> x) & static_cast<uint16_t>(1)) << y;
            }
        }
    }

    for (int z = 0; z < Chunk::DEPTH; ++z) {
        __m256i Y_plane = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&Y_rows[z][0]));

        __m256i up_faces = _mm256_and_si256(Y_plane, _mm256_andnot_si256(_mm256_srli_epi16(Y_plane, 1), _mm256_set1_epi16(0xFFFF)));
        __m256i down_faces = _mm256_and_si256(Y_plane, _mm256_andnot_si256(_mm256_slli_epi16(Y_plane, 1), _mm256_set1_epi16(0xFFFF)));

        uint16_t up_faces_plane[16], down_faces_plane[16];
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(up_faces_plane), up_faces);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(down_faces_plane), down_faces);

        for (int x = 0; x < Chunk::WIDTH; ++x) {
            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                face_planes_[2][y * Chunk::WIDTH + x] |= ((up_faces_plane[x] >> y) & static_cast<uint16_t>(1)) << z;
                face_planes_[3][y * Chunk::WIDTH + x] |= ((down_faces_plane[x] >> y) & static_cast<uint16_t>(1)) << z;
            }
        }
    }

    for (int x = 0; x < Chunk::WIDTH; ++x) {
        __m256i Z_plane = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&Z_rows[x][0]));

        __m256i right_faces = _mm256_and_si256(Z_plane, _mm256_andnot_si256(_mm256_srli_epi16(Z_plane, 1), _mm256_set1_epi16(0xFFFF)));
        __m256i left_faces = _mm256_and_si256(Z_plane, _mm256_andnot_si256(_mm256_slli_epi16(Z_plane, 1), _mm256_set1_epi16(0xFFFF)));

        uint16_t right_faces_plane[16], left_faces_plane[16];
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(right_faces_plane), right_faces);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(left_faces_plane), left_faces);

        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            for (int z = 0; z < Chunk::DEPTH; ++z) {
                face_planes_[4][z * Chunk::HEIGHT + y] |= ((right_faces_plane[y] >> z) & static_cast<uint16_t>(1)) << x;
                face_planes_[5][z * Chunk::HEIGHT + y] |= ((left_faces_plane[y] >> z) & static_cast<uint16_t>(1)) << x;
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
    uint64_t side1[VERTICES_COUNT_PER_SQUARE]{};
    uint64_t side2[VERTICES_COUNT_PER_SQUARE]{};
    uint64_t corner[VERTICES_COUNT_PER_SQUARE]{};

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
    } else if (direction == 5) {
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
    for (int i = 0; i < VERTICES_COUNT_PER_SQUARE; ++i) {
        result |= ((side2[i] + corner[i] + side1[i]) << (i * 2));
        result |= ((side2[i] << (10 + i * 3)) | (corner[i] << (9 + i * 3)) | (side1[i] << (8 + i * 3)));
    }

    return result;
}

// Good luck understanding this

void Chunk::GreedyMesh() {

    uint16_t is_processed[16];
    uint64_t type_data[16][16];

    uint16_t bit_mask;
    uint16_t next_bit_mask;
    uint16_t row_mask;

    uint16_t current_row;
    uint64_t current_type;
    uint64_t next_type;

    uint64_t brithness;
    uint64_t w, h;

    bool flag;

    uint64_t vertexAO[4];
    uint32_t vertex_index = 0;

    for (int direction = 0; direction < 6; ++direction) {
        if (direction < 2) {
            brithness = 14;
        } else if (direction == 2) {
            brithness = 15;
        } else if (direction == 3) {
            brithness = 12;
        } else {
            brithness = 13;
        }

        for (int plane = 0; plane < 16; ++plane) {

            std::fill(is_processed, is_processed + 16, 0);
            std::fill(&type_data[0][0], &type_data[0][0] + 256, 0);

            for (int row = 0; row < 16; ++row) {
                current_row = face_planes_[direction][plane * 16 + row];

                if (is_processed[row] == 0xFFFF || !current_row) {
                    continue;
                }

                for (int bit = 0; bit < 16; ++bit) {
                    bit_mask = (static_cast<uint16_t>(1) << bit);

                    if ((is_processed[row] & bit_mask) || !(current_row & bit_mask)) {
                        continue;
                    }

                    if (type_data[row][bit]) {
                        current_type = type_data[row][bit];
                    } else if (direction < 2) {
                        current_type =
                            ((AmbientOcclusion(plane, bit, row, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) |
                             (15ull << 8) | static_cast<uint64_t>(voxels_[(bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id));
                    } else if (direction < 4) {
                        current_type =
                            ((AmbientOcclusion(row, plane, bit, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) |
                             (15ull << 8) | static_cast<uint64_t>(voxels_[(plane * Chunk::DEPTH + bit) * Chunk::WIDTH + row].id));
                    } else {
                        current_type =
                            ((AmbientOcclusion(bit, row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) |
                             (15ull << 8) | static_cast<uint64_t>(voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + bit].id));
                    }
                    w = h = 1ull;
                    row_mask = bit_mask;
                    is_processed[row] |= bit_mask;


                    for (int next_bit = bit + 1; next_bit < 16; ++next_bit) {
                        next_bit_mask = (static_cast<uint16_t>(1) << next_bit);

                        if ((is_processed[row] & next_bit_mask) || !(current_row & next_bit_mask)) {
                            break;
                        }

                        if (type_data[row][next_bit]) {
                            next_type = type_data[row][next_bit];
                        } else if (direction < 2) {
                            next_type = ((AmbientOcclusion(plane, next_bit, row, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                         (brithness << 12) | (15ull << 8) |
                                         static_cast<uint64_t>(voxels_[(next_bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id));
                        } else if (direction < 4) {
                            next_type = ((AmbientOcclusion(row, plane, next_bit, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                         (brithness << 12) | (15ull << 8) |
                                         static_cast<uint64_t>(voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + row].id));
                        } else {
                            next_type = ((AmbientOcclusion(next_bit, row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                         (brithness << 12) | (15ull << 8) |
                                         static_cast<uint64_t>(voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id));
                        }

                        if (current_type != next_type) {
                            type_data[row][next_bit] = next_type;
                            break;
                        }

                        ++w;
                        row_mask |= next_bit_mask;
                        is_processed[row] |= next_bit_mask;
                    }


                    flag = false;
                    for (int next_row = row + 1; next_row < 16; ++next_row) {

                        if ((face_planes_[direction][plane * 16 + next_row] & row_mask) != row_mask) {
                            break;
                        }

                        for (int next_bit = bit; next_bit < bit + w; ++next_bit) {

                            if (type_data[next_row][next_bit]) {
                                next_type = type_data[next_row][next_bit];
                            } else if (direction < 2) {
                                next_type = ((AmbientOcclusion(plane, next_bit, next_row, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                             (brithness << 12) | (15ull << 8) |
                                             static_cast<uint64_t>(voxels_[(next_bit * Chunk::DEPTH + next_row) * Chunk::WIDTH + plane].id));
                            } else if (direction < 4) {
                                next_type = ((AmbientOcclusion(next_row, plane, next_bit, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                             (brithness << 12) | (15ull << 8) |
                                             static_cast<uint64_t>(voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + next_row].id));
                            } else {
                                next_type = ((AmbientOcclusion(next_bit, next_row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) |
                                             (brithness << 12) | (15ull << 8) |
                                             static_cast<uint64_t>(voxels_[(next_row * Chunk::DEPTH + plane) * Chunk::WIDTH + next_bit].id));
                            }

                            if (current_type != next_type) {
                                type_data[next_row][next_bit] = next_type;

                                for (int prev_bit = next_bit - 1; prev_bit >= bit; --prev_bit) {
                                    type_data[next_row][prev_bit] = current_type;
                                }

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

                    if (direction == 0) {
                        vertex_data.push_back((static_cast<uint64_t>(plane + 1) << 48) | (static_cast<uint64_t>(bit) << 43) |
                                              (static_cast<uint64_t>(row + h) << 38) | (2ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane + 1) << 48) | (static_cast<uint64_t>(bit) << 43) |
                                              (static_cast<uint64_t>(row) << 38) | (3ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane + 1) << 48) | (static_cast<uint64_t>(bit + w) << 43) |
                                              (static_cast<uint64_t>(row) << 38) | (1ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane + 1) << 48) | (static_cast<uint64_t>(bit + w) << 43) |
                                              (static_cast<uint64_t>(row + h) << 38) | (0ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 1) {
                        vertex_data.push_back((static_cast<uint64_t>(plane) << 48) | (static_cast<uint64_t>(bit) << 43) |
                                              (static_cast<uint64_t>(row) << 38) | (2ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane) << 48) | (static_cast<uint64_t>(bit) << 43) |
                                              (static_cast<uint64_t>(row + h) << 38) | (3ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane) << 48) | (static_cast<uint64_t>(bit + w) << 43) |
                                              (static_cast<uint64_t>(row + h) << 38) | (1ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane) << 48) | (static_cast<uint64_t>(bit + w) << 43) |
                                              (static_cast<uint64_t>(row) << 38) | (0ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 2) {
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 48) | (static_cast<uint64_t>(plane + 1) << 43) |
                                              (static_cast<uint64_t>(bit) << 38) | (1ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 48) | (static_cast<uint64_t>(plane + 1) << 43) |
                                              (static_cast<uint64_t>(bit) << 38) | (0ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 48) | (static_cast<uint64_t>(plane + 1) << 43) |
                                              (static_cast<uint64_t>(bit + w) << 38) | (2ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 48) | (static_cast<uint64_t>(plane + 1) << 43) |
                                              (static_cast<uint64_t>(bit + w) << 38) | (3ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 3) {
                        vertex_data.push_back((static_cast<uint64_t>(row) << 48) | (static_cast<uint64_t>(plane) << 43) |
                                              (static_cast<uint64_t>(bit) << 38) | (2ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 48) | (static_cast<uint64_t>(plane) << 43) |
                                              (static_cast<uint64_t>(bit) << 38) | (3ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 48) | (static_cast<uint64_t>(plane) << 43) |
                                              (static_cast<uint64_t>(bit + w) << 38) | (1ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 48) | (static_cast<uint64_t>(plane) << 43) |
                                              (static_cast<uint64_t>(bit + w) << 38) | (0ull << 36) | ((h - 1ull) << 32) | ((w - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 4) {
                        vertex_data.push_back((static_cast<uint64_t>(bit + w) << 48) | (static_cast<uint64_t>(row + h) << 43) |
                                              (static_cast<uint64_t>(plane + 1) << 38) | (1ull << 36) | ((w - 1ull) << 32) | ((h - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(bit) << 48) | (static_cast<uint64_t>(row + h) << 43) |
                                              (static_cast<uint64_t>(plane + 1) << 38) | (0ull << 36) | ((w - 1ull) << 32) | ((h - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(bit) << 48) | (static_cast<uint64_t>(row) << 43) |
                                              (static_cast<uint64_t>(plane + 1) << 38) | (2ull << 36) | ((w - 1ull) << 32) | ((h - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(bit + w) << 48) | (static_cast<uint64_t>(row) << 43) |
                                              (static_cast<uint64_t>(plane + 1) << 38) | (3ull << 36) | ((w - 1ull) << 32) | ((h - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 5) {
                        vertex_data.push_back((static_cast<uint64_t>(bit + w) << 48) | (static_cast<uint64_t>(row) << 43) |
                                              (static_cast<uint64_t>(plane) << 38) | (2ull << 36) | ((w - 1ull) << 32) | ((h - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(bit) << 48) | (static_cast<uint64_t>(row) << 43) |
                                              (static_cast<uint64_t>(plane) << 38) | (3ull << 36) | ((w - 1ull) << 32) | ((h - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(bit) << 48) | (static_cast<uint64_t>(row + h) << 43) |
                                              (static_cast<uint64_t>(plane) << 38) | (1ull << 36) | ((w - 1ull) << 32) | ((h - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(bit + w) << 48) | (static_cast<uint64_t>(row + h) << 43) |
                                              (static_cast<uint64_t>(plane) << 38) | (0ull << 36) | ((w - 1ull) << 32) | ((h - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                    }

                    if (vertexAO[0] + vertexAO[2] < vertexAO[1] + vertexAO[3]) {
                        index_data.push_back(vertex_index + 3);
                        index_data.push_back(vertex_index + 0);
                        index_data.push_back(vertex_index + 1);
                        index_data.push_back(vertex_index + 1);
                        index_data.push_back(vertex_index + 2);
                        index_data.push_back(vertex_index + 3);
                    } else {
                        index_data.push_back(vertex_index + 0);
                        index_data.push_back(vertex_index + 1);
                        index_data.push_back(vertex_index + 2);
                        index_data.push_back(vertex_index + 2);
                        index_data.push_back(vertex_index + 3);
                        index_data.push_back(vertex_index + 0);
                    }
                    vertex_index += Chunk::VERTICES_COUNT_PER_SQUARE;

                    bit += static_cast<int>(w) - 1;
                }
            }
        }
    }
}

Chunk::~Chunk() {

    delete[] face_planes_[0];
    delete[] voxels_;
}