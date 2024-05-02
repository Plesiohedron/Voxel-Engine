#include "Chunk.h"
#include "ChunkStorage.h"

#include <cmath>

#define IS_IN(X, Y, Z) ((X) >= 0 && (X) < Chunk::WIDTH && (Y) >= 0 && (Y) < Chunk::HEIGHT && (Z) >= 0 && (Z) < Chunk::DEPTH)
#define VOXEL(X, Y, Z) (voxels[((Y) * Chunk::DEPTH + (Z)) * Chunk::WIDTH + (X)])
#define IS_BLOCKED(X, Y, Z) ((IS_IN(X, Y, Z)) && VOXEL(X, Y, Z).id)

#define VERTEX(rgbs, u, v, x, y, z, id)\
    vertices.push_back(((x << 10) | (y << 5) | z));\
    vertices.push_back(((id << 10) | (u << 5) | v));\
    vertices.push_back(rgbs);

#define INDEX(VALUE)\
    indexes.push_back(VALUE + 0);\
    indexes.push_back(VALUE + 1);\
    indexes.push_back(VALUE + 2);\
    indexes.push_back(VALUE + 2);\
    indexes.push_back(VALUE + 1);\
    indexes.push_back(VALUE + 3);\
    VALUE += Chunk::VERTICES_COUNT_PER_SQUARE;

Chunk::Chunk(const glm::ivec3& coordinates)
    : global_coordinate_X(coordinates.x - ChunkStorage::storage_sizes.x / 2),
      global_coordinate_Y(coordinates.y),
      global_coordinate_Z(coordinates.z - ChunkStorage::storage_sizes.z / 2),

      local_coordinate_X(coordinates.x), local_coordinate_Y(coordinates.y), local_coordinate_Z(coordinates.z),

      mesh_data(3, std::vector<std::vector<std::vector<std::vector<uint32_t>>>>(
                 2, std::vector<std::vector<std::vector<uint32_t>>>(16, std::vector<std::vector<uint32_t>>(16, std::vector<uint32_t>(16))))) {

    vertices.reserve(6 * VERTEX_ATTRIBUTES_COUNT * (Chunk::WIDTH + 1) * (Chunk::HEIGHT + 1) * (Chunk::DEPTH + 1));
    indexes.reserve(6 * (Chunk::WIDTH + 1) * (Chunk::HEIGHT + 1) * (Chunk::DEPTH + 1));

    Chunk::VAO = std::make_unique<GL::VAO>();
    voxels = new Voxel[Chunk::VOLUME];

    uint8_t id;
    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                int global_x = x + global_coordinate_X * Chunk::WIDTH;
                int global_y = y + global_coordinate_Y * Chunk::DEPTH;
                int global_z = z + global_coordinate_Z * Chunk::HEIGHT;

                id = global_y <= std::sin(0.1 * global_x) * 10;
                if (global_y <= 2)
                    id = 4;

                // voxels[y][z][x] := voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]
                voxels[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id = id;
            }
        }
    }
}

Chunk::~Chunk() {
    delete[] voxels;
}

void Chunk::Render() {
    dataPlaneUpdate1.reserve(3 * 2 * 16);
    dataPlaneUpdate2.reserve(3 * 2 * 16);
    dataPlaneUpdate3.reserve(3 * 2 * 16);

    Culling();
    GreedyMeshing();

    dataPlaneUpdate1.clear();
    dataPlaneUpdate2.clear();
    dataPlaneUpdate3.clear();

    Chunk::VAO->Bind();
    Chunk::VAO->InitializeChunkVBO(Chunk::vertices);
    Chunk::VAO->InitializeEBO(Chunk::indexes);
    Chunk::VAO->PostInitialization();
}

void Chunk::Culling() {
    uint8_t id;

    unsigned short int brithness;

    for (int y = 0; y < Chunk::HEIGHT; ++y) {
        for (int z = 0; z < Chunk::DEPTH; ++z) {
            for (int x = 0; x < Chunk::WIDTH; ++x) {
                id = voxels[(y * Chunk::DEPTH + z) * Chunk::WIDTH + x].id;

                if (!id)
                    continue;

                if (!IS_BLOCKED(x - 1, y, z)) {
                    brithness = 13;
                    dataPlaneUpdate1.insert({((0 << 6) | (0 << 4) | x), true});
                    mesh_data[0][0][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x + 1, y, z)) {
                    brithness = 14;
                    dataPlaneUpdate1.insert({((0 << 6) | (1 << 4) | x), true});
                    mesh_data[0][1][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x, y - 1, z)) {
                    brithness = 11;
                    dataPlaneUpdate2.insert({((1 << 6) | (0 << 4) | y), true});
                    mesh_data[1][0][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x, y + 1, z)) {
                    brithness = 15;
                    dataPlaneUpdate2.insert({((1 << 6) | (1 << 4) | y), true});
                    mesh_data[1][1][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x, y, z - 1)) {
                    brithness = 12;
                    dataPlaneUpdate3.insert({((2 << 6) | (0 << 4) | z), true});
                    mesh_data[2][0][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }

                if (!IS_BLOCKED(x, y, z + 1)) {
                    brithness = 14;
                    dataPlaneUpdate3.insert({((2 << 6) | (1 << 4) | z), true});
                    mesh_data[2][1][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);
                }
            }
        }
    }
}

void Chunk::GreedyMeshing() {
    unsigned short int index = 0;
    bool massive[16][16]{false};

    for (const auto& [key, value] : dataPlaneUpdate1) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int i = (key & 0b00001111);


        std::fill(&massive[0][0], &massive[0][0] + std::size(massive) * std::size(massive[0]), false);

        for (int j = 0; j < 16; ++j) {
            for (int k = 0; k < 16; ++k) {

                if (mesh_data[p][q][i][j][k] == 0 || massive[j][k] == true) {
                    continue;
                }

                uint32_t currentFace = mesh_data[p][q][i][j][k];
                int repeatZ = 0;
                int repeatY = 0;

                ++repeatY;
                for (int v = k; v < 16; ++v) {
                    if (mesh_data[p][q][i][j][v] == currentFace) {
                        massive[j][v] = true;
                        ++repeatZ;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = j + 1; u < 16; ++u) {
                    for (int v = k; v < k + repeatZ; ++v) {
                        if (mesh_data[p][q][i][u][v] != currentFace) {
                            flag = true;
                            break;
                        }
                    }

                    if (flag) {
                        break;
                    } else {
                        ++repeatY;
                        for (int v = k; v < k + repeatZ; ++v) {
                            massive[u][v] = true;
                        }
                    }
                }

                if (q == 0) {
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0,           i, j + repeatY, k + repeatZ, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0,           i, j + repeatY, k,           (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0 + repeatY, i, j,           k + repeatZ, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatY, i, j,           k,           (mesh_data[p][q][i][j][k] >> 16));

                    INDEX(index);
                } else {
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0,           i + 1, j + repeatY, k + repeatZ, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatY, i + 1, j,           k + repeatZ, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0,           i + 1, j + repeatY, k,           (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0 + repeatY, i + 1, j,           k,           (mesh_data[p][q][i][j][k] >> 16));

                    INDEX(index);
                }
            }
        }
    }

    for (const auto& [key, value] : dataPlaneUpdate2) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int j = (key & 0b00001111);


        std::fill(&massive[0][0], &massive[0][0] + std::size(massive) * std::size(massive[0]), false);

        for (int i = 0; i < 16; ++i) {
            for (int k = 0; k < 16; ++k) {

                if (mesh_data[p][q][i][j][k] == 0 || massive[i][k] == true) {
                    continue;
                }

                uint32_t currentFace = mesh_data[p][q][i][j][k];
                int repeatZ = 0;
                int repeatX = 0;

                ++repeatX;
                for (int v = k; v < 16; ++v) {
                    if (mesh_data[p][q][i][j][v] == currentFace) {
                        massive[i][v] = true;
                        ++repeatZ;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = i + 1; u < 16; ++u) {
                    for (int v = k; v < k + repeatZ; ++v) {
                        if (mesh_data[p][q][u][j][v] != currentFace) {
                            flag = true;
                            break;
                        }
                    }

                    if (flag) {
                        break;
                    } else {
                        ++repeatX;
                        for (int v = k; v < k + repeatZ; ++v) {
                            massive[u][v] = true;
                        }
                    }
                }

                if (q == 0) {
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0,           i + repeatX, j, k + repeatZ, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0,           i,           j, k + repeatZ, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0 + repeatX, i + repeatX, j, k,           (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatX, i,           j, k,           (mesh_data[p][q][i][j][k] >> 16));

                    INDEX(index);
                } else {
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatX, 0 + repeatZ, i + repeatX, j + 1, k + repeatZ, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatX, 0,           i + repeatX, j + 1, k,           (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatZ, i,           j + 1, k + repeatZ, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0,           i,           j + 1, k,           (mesh_data[p][q][i][j][k] >> 16));

                    INDEX(index);
                }
            }
        }
    }

    for (const auto& [key, value] : dataPlaneUpdate3) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int k = (key & 0b00001111);


        std::fill(&massive[0][0], &massive[0][0] + std::size(massive) * std::size(massive[0]), false);

        for (int i = 0; i < 16; ++i) {
            for (int j = 0; j < 16; ++j) {

                if (mesh_data[p][q][i][j][k] == 0 || massive[i][j] == true) {
                    continue;
                }

                uint32_t currentFace = mesh_data[p][q][i][j][k];
                int repeatX = 0;
                int repeatY = 0;

                ++repeatY;
                for (int v = j; v < 16; ++v) {
                    if (mesh_data[p][q][i][v][k] == currentFace) {
                        massive[i][v] = true;
                        ++repeatX;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = i + 1; u < 16; ++u) {
                    for (int v = j; v < j + repeatX; ++v) {
                        if (mesh_data[p][q][u][v][k] != currentFace) {
                            flag = true;
                            break;
                        }
                    }

                    if (flag) {
                        break;
                    } else {
                        ++repeatY;
                        for (int v = j; v < j + repeatX; ++v) {
                            massive[u][v] = true;
                        }
                    }
                }

                if (q == 0) {
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0,           i + repeatY, j + repeatX, k, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatX, i + repeatY, j,           k, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatY, 0,           i,           j + repeatX, k, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatY, 0 + repeatX, i,           j,           k, (mesh_data[p][q][i][j][k] >> 16));

                    INDEX(index);
                } else {
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatY, 0,           i + repeatY, j + repeatX, k + 1, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0,           i,           j + repeatX, k + 1, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0 + repeatY, 0 + repeatX, i + repeatY, j,           k + 1, (mesh_data[p][q][i][j][k] >> 16));
                    VERTEX((mesh_data[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatX, i,           j,           k + 1, (mesh_data[p][q][i][j][k] >> 16));

                    INDEX(index);
                }
            }
        }
    }
}

void Chunk::Draw(const GLenum primitive_type) const {
    Chunk::VAO->Draw(primitive_type);
}
