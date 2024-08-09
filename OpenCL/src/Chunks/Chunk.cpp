#include "Chunk.h"

#include <immintrin.h>
#include <bitset>
#include <cmath>
#include <chrono>

Chunk::Chunk() {

    face_planes_[0] = new uint16_t[2 * (Chunk::WIDTH * Chunk::DEPTH + Chunk::HEIGHT * Chunk::WIDTH + Chunk::DEPTH * Chunk::HEIGHT)] {};

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

    uint16_t X_rows[Chunk::HEIGHT][Chunk::DEPTH] {};
    uint16_t Y_rows[Chunk::DEPTH][Chunk::WIDTH] {};
    uint16_t Z_rows[Chunk::WIDTH][Chunk::HEIGHT] {};

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                int global_x = x + global_coordinate_X * Chunk::WIDTH;
                int global_y = y + global_coordinate_Y * Chunk::DEPTH;
                int global_z = z + global_coordinate_Z * Chunk::HEIGHT;

                //uint8_t id = global_y <= std::sin(0.1 * global_x) * 10;
                //uint8_t id = global_y <= std::sin(0.1 * global_x) * 10 + std::cos(0.1 * global_z) * 10;
                /*uint8_t id = global_y <= 10;
                if (global_y <= 2) {
                    id = 4;
                }

                if (global_z > 2) {
                    id = 0;
                }

                if (global_x > 2) {
                    id = 0;
                }*/

                //uint8_t id = 1 + (global_x + global_y + global_z) % 2;
                //uint8_t id = 1 + (std::rand()) % 2;
                //uint8_t id = 0 + (std::rand()) % 2;
                uint8_t id = 0 + (global_x + global_y + global_z) % 2;

                //uint8_t id = 0;

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

    auto start = std::chrono::high_resolution_clock::now();
    Culling(X_rows, Y_rows, Z_rows);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Time taken by CPU Culling: " << duration.count() << " us\n\n";
}

void Chunk::Culling(uint16_t(&X_rows)[Chunk::HEIGHT][Chunk::DEPTH], uint16_t(&Y_rows)[Chunk::DEPTH][Chunk::WIDTH],
                    uint16_t(&Z_rows)[Chunk::WIDTH][Chunk::HEIGHT]) {

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            uint16_t forward_faces_row = X_rows[y][z] & ~(X_rows[y][z] >> 1);
            uint16_t back_faces_row = X_rows[y][z] & ~(X_rows[y][z] << 1);

            for (int x = 0; x < Chunk::WIDTH; ++x) {
                face_planes_[0][x * Chunk::DEPTH + z] |= ((forward_faces_row >> x) & 1) << y;
                face_planes_[1][x * Chunk::DEPTH + z] |= ((back_faces_row >> x) & 1) << y;
            }
        }
    }

    for (int z = 0; z < Chunk::DEPTH; ++z) {
        for (int x = 0; x < Chunk::WIDTH; ++x) {
            uint16_t up_faces_row = Y_rows[z][x] & ~(Y_rows[z][x] >> 1);
            uint16_t down_faces_row = Y_rows[z][x] & ~(Y_rows[z][x] << 1);

            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                face_planes_[2][y * Chunk::WIDTH + x] |= ((up_faces_row >> y) & 1) << z;
                face_planes_[3][y * Chunk::WIDTH + x] |= ((down_faces_row >> y) & 1) << z;
            }
        }
    }

    for (int x = 0; x < Chunk::WIDTH; ++x) {
        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            uint16_t right_faces_row = Z_rows[x][y] & ~(Z_rows[x][y] >> 1);
            uint16_t left_faces_row = Z_rows[x][y] & ~(Z_rows[x][y] << 1);

            for (int z = 0; z < Chunk::DEPTH; ++z) {
                face_planes_[4][z * Chunk::HEIGHT + y] |= ((right_faces_row >> z) & 1) << x;
                face_planes_[5][z * Chunk::HEIGHT + y] |= ((left_faces_row >> z) & 1) << x;
            }
        }
    }

}

bool Chunk::IsBlocked(int x, int y, int z) {
    if (x < 0 || x >= 16 || y < 0 || y >= 16 || z < 0 || z >= 16) {
        return false;
    }

    // TODO Add neighboring chunks

    return voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id;
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
        corner[0] = IsBlocked(x + 1, y + 1, z - 1);
        side2[0] = side1[1] = IsBlocked(x, y + 1, z - 1);
        corner[1] = IsBlocked(x - 1, y + 1, z - 1);
        side2[1] = side1[2] = IsBlocked(x - 1, y, z - 1);
        corner[2] = IsBlocked(x - 1, y - 1, z - 1);
        side2[2] = side1[3] = IsBlocked(x, y - 1, z - 1);
        corner[3] = IsBlocked(x + 1, y - 1, z - 1);
        side2[3] = side1[0] = IsBlocked(x + 1, y, z - 1);
    } else if (direction == 5) {
        corner[0] = IsBlocked(x - 1, y + 1, z + 1);
        side2[0] = side1[1] = IsBlocked(x, y + 1, z + 1);
        corner[1] = IsBlocked(x + 1, y + 1, z + 1);
        side2[1] = side1[2] = IsBlocked(x + 1, y, z + 1);
        corner[2] = IsBlocked(x + 1, y - 1, z + 1);
        side2[2] = side1[3] = IsBlocked(x, y - 1, z + 1);
        corner[3] = IsBlocked(x - 1, y - 1, z + 1);
        side2[3] = side1[0] = IsBlocked(x - 1, y, z + 1);
    }

    uint64_t result = 0;
    for (int i = 0; i < VERTICES_COUNT_PER_SQUARE; ++i) {
        result |= ((side2[i] + corner[i] + side1[i]) << (i * 2));
        result |= ((side2[i] << (10 + i * 3)) | (corner[i] << (9 + i * 3)) | (side1[i] << (8 + i * 3)));
    }

    return result;
}

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
            brithness = 13;
        } else if (direction == 2) {
            brithness = 15;
        } else if (direction == 3) {
            brithness = 9;
        } else {
            brithness = 11;
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
                        current_type = ((AmbientOcclusion(plane, bit, row, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
                                        static_cast<uint64_t>(voxels_[(bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id));
                    } else if (direction < 4) {
                        current_type = ((AmbientOcclusion(row, plane, bit, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
                                        static_cast<uint64_t>(voxels_[(plane * Chunk::DEPTH + bit) * Chunk::WIDTH + row].id));
                    } else {
                        current_type = ((AmbientOcclusion(bit, row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
                                        static_cast<uint64_t>(voxels_[(row * Chunk::DEPTH + plane) * Chunk::WIDTH + bit].id));
                    }
                    w = h = 1;
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
                            next_type = ((AmbientOcclusion(plane, next_bit, row, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
                                         static_cast<uint64_t>(voxels_[(next_bit * Chunk::DEPTH + row) * Chunk::WIDTH + plane].id));
                        } else if (direction < 4) {
                            next_type = ((AmbientOcclusion(row, plane, next_bit, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
                                         static_cast<uint64_t>(voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + row].id));
                        } else {
                            next_type = ((AmbientOcclusion(next_bit, row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
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
                                next_type = ((AmbientOcclusion(plane, next_bit, next_row, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
                                             static_cast<uint64_t>(voxels_[(next_bit * Chunk::DEPTH + next_row) * Chunk::WIDTH + plane].id));
                            } else if (direction < 4) {
                                next_type = ((AmbientOcclusion(next_row, plane, next_bit, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
                                             static_cast<uint64_t>(voxels_[(plane * Chunk::DEPTH + next_bit) * Chunk::WIDTH + next_row].id));
                            } else {
                                next_type = ((AmbientOcclusion(next_bit, next_row, plane, direction) << 24) | (brithness << 20) | (brithness << 16) | (brithness << 12) | (15ull << 8) |
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

                    //std::cout << current_type << ' ';

                    vertexAO[0] = (current_type >> 24) & 3;
                    vertexAO[1] = (current_type >> 26) & 3;
                    vertexAO[2] = (current_type >> 26) & 3;
                    vertexAO[3] = (current_type >> 30) & 3;

                    //std::cout << current_type << ' ';

                    if (direction == 0) {
                        vertex_data.push_back((static_cast<uint64_t>(plane + 1) << 46) | (static_cast<uint64_t>(bit + w) << 41) |
                                              (static_cast<uint64_t>(row + h) << 36) | (0ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                        std::cout << ((vertex_data.back() >> 46) & 31) << ' ' << ((vertex_data.back() >> 41) & 31) << ' ' << ((vertex_data.back() >> 36) & 31) << ' ' << ((vertex_data.back() >> 34) & 3) << ' ' << ((vertex_data.back() >> 30) & 0xF) + 1 << ' ' << ((vertex_data.back() >> 26) & 0xF) + 1 << ' ' << ((vertex_data.back() >> 24) & 3) << ' ' << ((vertex_data.back() >> 20) & 0xF) << ' ' << ((vertex_data.back() >> 16) & 0xF) << ' ' << ((vertex_data.back() >> 12) & 0xF) << ' ' << ((vertex_data.back() >> 8) & 0xF) << ' ' << (vertex_data.back() & 0xFF) << '\n';
                        vertex_data.push_back((static_cast<uint64_t>(plane + 1) << 46) | (static_cast<uint64_t>(bit + w) << 41) |
                                              (static_cast<uint64_t>(row) << 36) | (1ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        std::cout << ((vertex_data.back() >> 46) & 31) << ' ' << ((vertex_data.back() >> 41) & 31) << ' ' << ((vertex_data.back() >> 36) & 31) << ' ' << ((vertex_data.back() >> 34) & 3) << ' ' << ((vertex_data.back() >> 30) & 0xF) + 1 << ' ' << ((vertex_data.back() >> 26) & 0xF) + 1 << ' ' << ((vertex_data.back() >> 24) & 3) << ' ' << ((vertex_data.back() >> 20) & 0xF) << ' ' << ((vertex_data.back() >> 16) & 0xF) << ' ' << ((vertex_data.back() >> 12) & 0xF) << ' ' << ((vertex_data.back() >> 8) & 0xF) << ' ' << (vertex_data.back() & 0xFF) << '\n';
                        vertex_data.push_back((static_cast<uint64_t>(plane + 1) << 46) | (static_cast<uint64_t>(bit) << 41) |
                                              (static_cast<uint64_t>(row) << 36) | (2ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        std::cout << ((vertex_data.back() >> 46) & 31) << ' ' << ((vertex_data.back() >> 41) & 31) << ' ' << ((vertex_data.back() >> 36) & 31) << ' ' << ((vertex_data.back() >> 34) & 3) << ' ' << ((vertex_data.back() >> 30) & 0xF) + 1 << ' ' << ((vertex_data.back() >> 26) & 0xF) + 1 << ' ' << ((vertex_data.back() >> 24) & 3) << ' ' << ((vertex_data.back() >> 20) & 0xF) << ' ' << ((vertex_data.back() >> 16) & 0xF) << ' ' << ((vertex_data.back() >> 12) & 0xF) << ' ' << ((vertex_data.back() >> 8) & 0xF) << ' ' << (vertex_data.back() & 0xFF) << '\n';
                        vertex_data.push_back((static_cast<uint64_t>(plane + 1) << 46) | (static_cast<uint64_t>(bit) << 41) |
                                              (static_cast<uint64_t>(row + h) << 36) | (3ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                        std::cout << ((vertex_data.back() >> 46) & 31) << ' ' << ((vertex_data.back() >> 41) & 31) << ' ' << ((vertex_data.back() >> 36) & 31) << ' ' << ((vertex_data.back() >> 34) & 3) << ' ' << ((vertex_data.back() >> 30) & 0xF) + 1 << ' ' << ((vertex_data.back() >> 26) & 0xF) + 1 << ' ' << ((vertex_data.back() >> 24) & 3) << ' ' << ((vertex_data.back() >> 20) & 0xF) << ' ' << ((vertex_data.back() >> 16) & 0xF) << ' ' << ((vertex_data.back() >> 12) & 0xF) << ' ' << ((vertex_data.back() >> 8) & 0xF) << ' ' << (vertex_data.back() & 0xFF) << '\n';
                    } else if (direction == 1) {
                        vertex_data.push_back((static_cast<uint64_t>(plane) << 46) | (static_cast<uint64_t>(bit + w) << 41) |
                                              (static_cast<uint64_t>(row) << 36) | (0ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane) << 46) | (static_cast<uint64_t>(bit + w) << 41) |
                                              (static_cast<uint64_t>(row + h) << 36) | (1ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane) << 46) | (static_cast<uint64_t>(bit) << 41) |
                                              (static_cast<uint64_t>(row + h) << 36) | (2ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(plane) << 46) | (static_cast<uint64_t>(bit) << 41) |
                                              (static_cast<uint64_t>(row) << 36) | (3ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 2) {
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 46) | (static_cast<uint64_t>(plane + 1) << 41) |
                                              (static_cast<uint64_t>(bit + w) << 36) | (0ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 46) | (static_cast<uint64_t>(plane + 1) << 41) |
                                              (static_cast<uint64_t>(bit + w) << 36) | (1ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 46) | (static_cast<uint64_t>(plane + 1) << 41) |
                                              (static_cast<uint64_t>(bit) << 36) | (2ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 46) | (static_cast<uint64_t>(plane + 1) << 41) |
                                              (static_cast<uint64_t>(bit) << 36) | (3ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 3) {
                        vertex_data.push_back((static_cast<uint64_t>(row) << 46) | (static_cast<uint64_t>(plane) << 41) |
                                              (static_cast<uint64_t>(bit + w) << 36) | (0ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 46) | (static_cast<uint64_t>(plane) << 41) |
                                              (static_cast<uint64_t>(bit + w) << 36) | (1ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 46) | (static_cast<uint64_t>(plane) << 41) |
                                              (static_cast<uint64_t>(bit) << 36) | (2ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 46) | (static_cast<uint64_t>(plane) << 41) |
                                              (static_cast<uint64_t>(bit) << 36) | (3ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 4) {
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 46) | (static_cast<uint64_t>(bit) << 41) |
                                              (static_cast<uint64_t>(plane + 1) << 36) | (0ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 46) | (static_cast<uint64_t>(bit + w) << 41) |
                                              (static_cast<uint64_t>(plane + 1) << 36) | (1ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 46) | (static_cast<uint64_t>(bit + w) << 41) |
                                              (static_cast<uint64_t>(plane + 1) << 36) | (2ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 46) | (static_cast<uint64_t>(bit) << 41) |
                                              (static_cast<uint64_t>(plane + 1) << 36) | (3ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                    } else if (direction == 5) {
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 46) | (static_cast<uint64_t>(bit + w) << 41) |
                                              (static_cast<uint64_t>(plane) << 36) | (0ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[0] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row + h) << 46) | (static_cast<uint64_t>(bit) << 41) |
                                              (static_cast<uint64_t>(plane) << 36) | (1ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[1] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 46) | (static_cast<uint64_t>(bit) << 41) |
                                              (static_cast<uint64_t>(plane) << 36) | (2ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[2] << 24) | (current_type & 0x0000000000FFFFFF));
                        vertex_data.push_back((static_cast<uint64_t>(row) << 46) | (static_cast<uint64_t>(bit + w) << 41) |
                                              (static_cast<uint64_t>(plane) << 36) | (3ull << 34) | ((w - 1ull) << 30) | ((h - 1ull) << 26) |
                                              (vertexAO[3] << 24) | (current_type & 0x0000000000FFFFFF));
                    }

                    if (vertexAO[0] + vertexAO[2] > vertexAO[1] + vertexAO[3]) {
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