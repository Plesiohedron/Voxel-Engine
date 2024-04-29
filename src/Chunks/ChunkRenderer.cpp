#include "ChunkRenderer.h"
#include <iostream>

std::unordered_map<uint8_t, bool> ChunkRenderer::dataPlaneUpdate1;
std::unordered_map<uint8_t, bool> ChunkRenderer::dataPlaneUpdate2;
std::unordered_map<uint8_t, bool> ChunkRenderer::dataPlaneUpdate3;

#define IS_IN(X, Y, Z) ((X) >= 0 && (X) < CHUNK_W && (Y) >= 0 && (Y) < CHUNK_H && (Z) >= 0 && (Z) < CHUNK_D)
#define VOXEL(X, Y, Z) (chunk.voxels[((Y) * CHUNK_D + (Z)) * CHUNK_W + (X)])
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
    VALUE += VERTICES_COUNT_PER_SQUARE;

void ChunkRenderer::Render(Chunk& chunk, std::vector<GLushort>& vertices, std::vector<GLushort>& indexes) {
    dataPlaneUpdate1.reserve(3 * 2 * 16);
    dataPlaneUpdate2.reserve(3 * 2 * 16);

    uint8_t id;

    GLushort brithness;

    for (GLushort y = 0; y < CHUNK_H; ++y) {
        for (GLushort z = 0; z < CHUNK_D; ++z) {
            for (GLushort x = 0; x < CHUNK_W; ++x) {
                id = chunk.voxels[(y * CHUNK_D + z) * CHUNK_W + x].id;

                if (!id)
                    continue;

                if (!IS_BLOCKED(x - 1, y, z)) {
                    brithness = 13;

                    dataPlaneUpdate1.insert({((0 << 6) | (0 << 4) | x), true});

                    chunk.meshData[0][0][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);

                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, 0 + 1, 0,     x, y + 1, z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, 0,     0,     x, y + 1, z,     id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, 0 + 1, 0 + 1, x, y,     z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, 0,     0 + 1, x, y,     z,     id);

                    //INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x + 1, y, z)) {
                    brithness = 14;

                    dataPlaneUpdate1.insert({((0 << 6) | (1 << 4) | x), true});

                    chunk.meshData[0][1][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);

                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x + 1, y + 1, z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x + 1, y,     z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x + 1, y + 1, z,     id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x + 1, y,     z,     id);

                    //INDEX(chunk.currentIndexesCount, index);
                }
                
                if (!IS_BLOCKED(x, y - 1, z)) {
                    brithness = 11;

                    dataPlaneUpdate2.insert({((1 << 6) | (0 << 4) | y), true});

                    chunk.meshData[1][0][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);

                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x + 1, y, z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x,     y, z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x + 1, y, z,     id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x,     y, z,     id);

                    //INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x, y + 1, z)) {
                    brithness = 15;

                    dataPlaneUpdate2.insert({((1 << 6) | (1 << 4) | y), true});

                    chunk.meshData[1][1][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);

                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x + 1, y + 1, z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x + 1, y + 1, z,     id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x,     y + 1, z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x,     y + 1, z,     id);

                    //INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x, y, z - 1)) {
                    brithness = 12;

                    dataPlaneUpdate3.insert({((2 << 6) | (0 << 4) | z), true});

                    chunk.meshData[2][0][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);

                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x + 1, y + 1, z, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x + 1, y,     z, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x,     y + 1, z, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x,     y,     z, id);

                    //INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x, y, z + 1)) {
                    brithness = 14;

                    dataPlaneUpdate3.insert({((2 << 6) | (1 << 4) | z), true});

                    chunk.meshData[2][1][x][y][z] = ((id << 16) | (brithness << 12) | (brithness << 8) | (brithness << 4) | 15);

                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x + 1, y + 1, z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x,     y + 1, z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x + 1, y,     z + 1, id);
                    //VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x,     y,     z + 1, id);

                    //INDEX(chunk.currentIndexesCount, index);
                }
            }
        }
    }

    GreedyMesh(chunk, vertices, indexes);
}

void ChunkRenderer::GreedyMesh(const Chunk& chunk, std::vector<GLushort>& vertices, std::vector<GLushort>& indexes) {
    GLushort index = 0;
    bool massive[16][16]{false};

    for (const auto& [key, value] : dataPlaneUpdate1) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int i = (key & 0b00001111);


        std::fill(&massive[0][0], &massive[0][0] + std::size(massive) * std::size(massive[0]), false);

        for (int j = 0; j < 16; ++j) {
            for (int k = 0; k < 16; ++k) {

                if (chunk.meshData[p][q][i][j][k] == 0 || massive[j][k] == true) {
                    continue;
                }

                uint32_t currentFace = chunk.meshData[p][q][i][j][k];
                int repeatZ = 0;
                int repeatY = 0;

                ++repeatY;
                for (int v = k; v < 16; ++v) {
                    if (chunk.meshData[p][q][i][j][v] == currentFace) {
                        massive[j][v] = true;
                        ++repeatZ;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = j + 1; u < 16; ++u) {
                    for (int v = k; v < k + repeatZ; ++v) {
                        if (chunk.meshData[p][q][i][u][v] != currentFace) {
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
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0,           i, j + repeatY, k + repeatZ, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0,           i, j + repeatY, k,           (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0 + repeatY, i, j,           k + repeatZ, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatY, i, j,           k,           (chunk.meshData[p][q][i][j][k] >> 16));

                    INDEX(index);
                } else {
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0,           i + 1, j + repeatY, k + repeatZ, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatY, i + 1, j,           k + repeatZ, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0,           i + 1, j + repeatY, k,           (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0 + repeatY, i + 1, j,           k,           (chunk.meshData[p][q][i][j][k] >> 16));

                    INDEX(index);
                }
            }
        }
    }

    dataPlaneUpdate1.clear();

    for (const auto& [key, value] : dataPlaneUpdate2) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int j = (key & 0b00001111);


        std::fill(&massive[0][0], &massive[0][0] + std::size(massive) * std::size(massive[0]), false);

        for (int i = 0; i < 16; ++i) {
            for (int k = 0; k < 16; ++k) {

                if (chunk.meshData[p][q][i][j][k] == 0 || massive[i][k] == true) {
                    continue;
                }

                uint32_t currentFace = chunk.meshData[p][q][i][j][k];
                int repeatZ = 0;
                int repeatX = 0;

                ++repeatX;
                for (int v = k; v < 16; ++v) {
                    if (chunk.meshData[p][q][i][j][v] == currentFace) {
                        massive[i][v] = true;
                        ++repeatZ;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = i + 1; u < 16; ++u) {
                    for (int v = k; v < k + repeatZ; ++v) {
                        if (chunk.meshData[p][q][u][j][v] != currentFace) {
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
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0,           i + repeatX, j, k + repeatZ, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0,           i,           j, k + repeatZ, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatZ, 0 + repeatX, i + repeatX, j, k,           (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatX, i,           j, k,           (chunk.meshData[p][q][i][j][k] >> 16));

                    INDEX(index);
                } else {
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatX, 0 + repeatZ, i + repeatX, j + 1, k + repeatZ, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatX, 0,           i + repeatX, j + 1, k,           (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatZ, i,           j + 1, k + repeatZ, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0,           i,           j + 1, k,           (chunk.meshData[p][q][i][j][k] >> 16));

                    INDEX(index);
                }
            }
        }
    }

    dataPlaneUpdate2.clear();

    for (const auto& [key, value] : dataPlaneUpdate3) {
        int p = ((key >> 6) & 0b00000011);
        int q = ((key >> 4) & 0b00000011);

        int k = (key & 0b00001111);


        std::fill(&massive[0][0], &massive[0][0] + std::size(massive) * std::size(massive[0]), false);

        for (int i = 0; i < 16; ++i) {
            for (int j = 0; j < 16; ++j) {

                if (chunk.meshData[p][q][i][j][k] == 0 || massive[i][j] == true) {
                    continue;
                }

                uint32_t currentFace = chunk.meshData[p][q][i][j][k];
                int repeatX = 0;
                int repeatY = 0;

                ++repeatY;
                for (int v = j; v < 16; ++v) {
                    if (chunk.meshData[p][q][i][v][k] == currentFace) {
                        massive[i][v] = true;
                        ++repeatX;
                    } else {
                        break;
                    }
                }

                bool flag = false;

                for (int u = i + 1; u < 16; ++u) {
                    for (int v = j; v < j + repeatX; ++v) {
                        if (chunk.meshData[p][q][u][v][k] != currentFace) {
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
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0,           i + repeatY, j + repeatX, k, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatX, i + repeatY, j,           k, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatY, 0,           i,           j + repeatX, k, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatY, 0 + repeatX, i,           j,           k, (chunk.meshData[p][q][i][j][k] >> 16));

                    INDEX(index);
                } else {
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatY, 0,           i + repeatY, j + repeatX, k + 1, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0,           i,           j + repeatX, k + 1, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0 + repeatY, 0 + repeatX, i + repeatY, j,           k + 1, (chunk.meshData[p][q][i][j][k] >> 16));
                    VERTEX((chunk.meshData[p][q][i][j][k] & 0xFFFF), 0,           0 + repeatX, i,           j,           k + 1, (chunk.meshData[p][q][i][j][k] >> 16));

                    INDEX(index);
                }
            }
        }
    }

    dataPlaneUpdate3.clear();
}
