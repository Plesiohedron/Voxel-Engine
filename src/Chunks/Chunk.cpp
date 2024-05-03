#include "Chunk.h"
#include "ChunkStorage.h"

#define IS_IN(X, Y, Z) ((X) >= 0 && (X) < Chunk::WIDTH && (Y) >= 0 && (Y) < Chunk::HEIGHT && (Z) >= 0 && (Z) < Chunk::DEPTH)
#define VOXEL(X, Y, Z) (voxels_[((Y) * Chunk::DEPTH + (Z)) * Chunk::WIDTH + (X)])
#define IS_BLOCKED(X, Y, Z) ((IS_IN(X, Y, Z)) && VOXEL(X, Y, Z).id)

#define VERTEX(rgbs, u, v, x, y, z, id)\
    vertices[vertices_array_size + 0] = ((x << 10) | (y << 5) | z);\
    vertices[vertices_array_size + 1] = ((id << 10) | (u << 5) | v);\
    vertices[vertices_array_size + 2] = (rgbs);\
    vertices_array_size += VERTEX_ATTRIBUTES_COUNT;

#define INDEX(VALUE)\
    indexes[indexes_array_size + 0] = VALUE + 0;\
    indexes[indexes_array_size + 1] = VALUE + 1;\
    indexes[indexes_array_size + 2] = VALUE + 2;\
    indexes[indexes_array_size + 3] = VALUE + 2;\
    indexes[indexes_array_size + 4] = VALUE + 1;\
    indexes[indexes_array_size + 5] = VALUE + 3;\
    indexes_array_size += Chunk::INDEXES_COUNT_PER_SQUARE;\
    VALUE += Chunk::VERTICES_COUNT_PER_SQUARE;

Chunk::Chunk(const glm::ivec3& coordinates)
    : global_coordinate_X(coordinates.x - ChunkStorage::storage_sizes.x / 2),
      global_coordinate_Y(coordinates.y),
      global_coordinate_Z(coordinates.z - ChunkStorage::storage_sizes.z / 2),

      //local_coordinate_X(coordinates.x), local_coordinate_Y(coordinates.y), local_coordinate_Z(coordinates.z),

      // 3 dimensions, 2 directions, 16 planes, 16x16 blocks per plane
      mesh_data_({3, 2, 16, 16, 16}) {

    vertices = new unsigned short int[6 * VERTEX_ATTRIBUTES_COUNT * (Chunk::WIDTH + 1) * (Chunk::HEIGHT + 1) * (Chunk::DEPTH + 1)];
    indexes = new unsigned short int[6 * (Chunk::WIDTH + 1) * (Chunk::HEIGHT + 1) * (Chunk::DEPTH + 1)];

    voxels_ = new Voxel[Chunk::VOLUME];

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                int global_x = x + global_coordinate_X * Chunk::WIDTH;
                int global_y = y + global_coordinate_Y * Chunk::DEPTH;
                int global_z = z + global_coordinate_Z * Chunk::HEIGHT;

                uint8_t id = global_y <= std::sin(0.1 * global_x) * 10;
                if (global_y <= 2) {
                    id = 4;
                }

                // voxels[y][z][x] := voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]
                voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id = id;
            }
        }
    }
}

void Chunk::Render() {
    Culling();
    GreedyMeshing();
}

void Chunk::Culling() {
    unsigned short int brithness;

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                uint8_t id = voxels_[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id;

                if (!id) {
                    continue;
                }

                if (!IS_BLOCKED(x - 1, y, z)) {
                    brithness = 13;
                    //ChunkStorage::X_planes_.push_back(((0 << 6) | (0 << 4) | x));
                    ChunkStorage::X_planes_[((0 << 6) | (0 << 4) | x)] = true;
                    mesh_data_({0, 0, x, y, z}) = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x + 1, y, z)) {
                    brithness = 14;
                    //ChunkStorage::X_planes_.push_back(((0 << 6) | (1 << 4) | x));
                    ChunkStorage::X_planes_[((0 << 6) | (1 << 4) | x)] = true;
                    mesh_data_({0, 1, x, y, z}) = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x, y - 1, z)) {
                    brithness = 11;
                    //ChunkStorage::Y_planes_.push_back(((1 << 6) | (0 << 4) | y));
                    ChunkStorage::Y_planes_[((1 << 6) | (0 << 4) | y)] = true;
                    mesh_data_({1, 0, x, y, z}) = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x, y + 1, z)) {
                    brithness = 15;
                    //ChunkStorage::Y_planes_.push_back(((1 << 6) | (1 << 4) | y));
                    ChunkStorage::Y_planes_[((1 << 6) | (1 << 4) | y)] = true;
                    mesh_data_({1, 1, x, y, z}) = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x, y, z - 1)) {
                    brithness = 12;
                    //ChunkStorage::Z_planes_.push_back(((2 << 6) | (0 << 4) | z));
                    ChunkStorage::Z_planes_[((2 << 6) | (0 << 4) | z)] = true;
                    mesh_data_({2, 0, x, y, z}) = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x, y, z + 1)) {
                    brithness = 14;
                    //ChunkStorage::Z_planes_.push_back(((2 << 6) | (1 << 4) | z));
                    ChunkStorage::Z_planes_[((2 << 6) | (1 << 4) | z)] = true;
                    mesh_data_({2, 1, x, y, z}) = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }
            }
        }
    }
}

// Good luck undertand this

void Chunk::GreedyMeshing() {
    unsigned short int index = 0;
    bool face_is_merged[16][16]{false};

    for (const auto& [key, value] : ChunkStorage::X_planes_) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int i = (key & 0b00001111);


        std::fill(&face_is_merged[0][0], &face_is_merged[0][0] + std::size(face_is_merged) * std::size(face_is_merged[0]), false);

        for (int j = 0; j < 16; ++j) {
            for (int k = 0; k < 16; ++k) {

                if (mesh_data_({p, q, i, j, k}) == 0 || face_is_merged[j][k] == true) {
                    continue;
                }

                uint32_t current_face = mesh_data_({p, q, i, j, k});
                int max_width = 0;  // -> Z
                int max_height = 0;  // -> Y

                ++max_height;
                for (int v = k; v < 16; ++v) {
                    if (mesh_data_({p, q, i, j, v}) == current_face) {
                        face_is_merged[j][v] = true;
                        ++max_width;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = j + 1; u < 16; ++u) {
                    for (int v = k; v < k + max_width; ++v) {
                        if (mesh_data_({p, q, i, u, v}) != current_face) {
                            flag = true;
                            break;
                        }
                    }

                    if (flag) {
                        break;
                    } else {
                        ++max_height;
                        for (int v = k; v < k + max_width; ++v) {
                            face_is_merged[u][v] = true;
                        }
                    }
                }

                if (q == 0) {
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0,              i, j + max_height, k + max_width, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0,              i, j + max_height, k,             (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0 + max_height, i, j,              k + max_width, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0 + max_height, i, j,              k,             (mesh_data_({p, q, i, j, k}) >> 16));

                    INDEX(index);
                } else {
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0,              i + 1, j + max_height, k + max_width, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0 + max_height, i + 1, j,              k + max_width, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0,              i + 1, j + max_height, k,             (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0 + max_height, i + 1, j,              k,             (mesh_data_({p, q, i, j, k}) >> 16));

                    INDEX(index);
                }
            }
        }
    }

    for (const auto& [key, value] : ChunkStorage::Y_planes_) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int j = (key & 0b00001111);


        std::fill(&face_is_merged[0][0], &face_is_merged[0][0] + std::size(face_is_merged) * std::size(face_is_merged[0]), false);

        for (int i = 0; i < 16; ++i) {
            for (int k = 0; k < 16; ++k) {

                if (mesh_data_({p, q, i, j, k}) == 0 || face_is_merged[i][k] == true) {
                    continue;
                }

                uint32_t current_face = mesh_data_({p, q, i, j, k});
                int max_width = 0;  // -> Z
                int max_height = 0;  // -> X

                ++max_height;
                for (int v = k; v < 16; ++v) {
                    if (mesh_data_({p, q, i, j, v}) == current_face) {
                        face_is_merged[i][v] = true;
                        ++max_width;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = i + 1; u < 16; ++u) {
                    for (int v = k; v < k + max_width; ++v) {
                        if (mesh_data_({p, q, u, j, v}) != current_face) {
                            flag = true;
                            break;
                        }
                    }

                    if (flag) {
                        break;
                    } else {
                        ++max_height;
                        for (int v = k; v < k + max_width; ++v) {
                            face_is_merged[u][v] = true;
                        }
                    }
                }

                if (q == 0) {
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0,              i + max_height, j, k + max_width, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0,              i,              j, k + max_width, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0 + max_height, i + max_height, j, k,             (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0 + max_height, i,              j, k,             (mesh_data_({p, q, i, j, k}) >> 16));

                    INDEX(index);
                } else {
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_height, 0 + max_width, i + max_height, j + 1, k + max_width, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_height, 0,             i + max_height, j + 1, k,             (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,              0 + max_width, i,              j + 1, k + max_width, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,              0,             i,              j + 1, k,             (mesh_data_({p, q, i, j, k}) >> 16));

                    INDEX(index);
                }
            }
        }
    }

    for (const auto& [key, value] : ChunkStorage::Z_planes_) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int k = (key & 0b00001111);


        std::fill(&face_is_merged[0][0], &face_is_merged[0][0] + std::size(face_is_merged) * std::size(face_is_merged[0]), false);

        for (int i = 0; i < 16; ++i) {
            for (int j = 0; j < 16; ++j) {

                if (mesh_data_({p, q, i, j, k}) == 0 || face_is_merged[i][j] == true) {
                    continue;
                }

                uint32_t current_face = mesh_data_({p, q, i, j, k});
                int max_height = 0;  // -> Y
                int max_width = 0;  // -> X

                ++max_width;
                for (int v = j; v < 16; ++v) {
                    if (mesh_data_({p, q, i, v, k}) == current_face) {
                        face_is_merged[i][v] = true;
                        ++max_height;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = i + 1; u < 16; ++u) {
                    for (int v = j; v < j + max_height; ++v) {
                        if (mesh_data_({p, q, u, v, k}) != current_face) {
                            flag = true;
                            break;
                        }
                    }

                    if (flag) {
                        break;
                    } else {
                        ++max_width;
                        for (int v = j; v < j + max_height; ++v) {
                            face_is_merged[u][v] = true;
                        }
                    }
                }

                if (q == 0) {
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0,              i + max_width, j + max_height, k, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0 + max_height, i + max_width, j,              k, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0,              i,             j + max_height, k, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0 + max_height, i,             j,              k, (mesh_data_({p, q, i, j, k}) >> 16));

                    INDEX(index);
                } else {
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0,              i + max_width, j + max_height, k + 1, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0,              i,             j + max_height, k + 1, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0 + max_width, 0 + max_height, i + max_width, j,              k + 1, (mesh_data_({p, q, i, j, k}) >> 16));
                    VERTEX((mesh_data_({p, q, i, j, k}) & 0xFFFF), 0,             0 + max_height, i,             j,              k + 1, (mesh_data_({p, q, i, j, k}) >> 16));

                    INDEX(index);
                }
            }
        }
    }
}

Chunk::~Chunk() {
    delete[] voxels_;
}