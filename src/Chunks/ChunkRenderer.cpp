#include "ChunkRenderer.h"

#include <iostream>

#define YZ_IS_IN(Y, Z) ((Y) >= 0 && (Y) < CHUNK_H && (Z) >= 0 && (Z) < CHUNK_D)
#define XY_IS_IN(X, Y) ((X) >= 0 && (X) < CHUNK_W && (Y) >= 0 && (Y) < CHUNK_H)
#define Y_IS_IN(Y) ((Y) >= 0 && (Y) < CHUNK_H)
#define IS_IN(X,Y,Z) ((X) >= 0 && (X) < CHUNK_W && (Y) >= 0 && (Y) < CHUNK_H && (Z) >= 0 && (Z) < CHUNK_D)

#define IS_BLOCKED(X,Y,Z) (IS_IN(X,Y,Z) ? chunk.voxels[((Y) * CHUNK_D + (Z)) * CHUNK_W + (X)].id : 0)

#define Q4(X,Y,Z) (((X) == -1 && YZ_IS_IN(Y, Z) && (neighbouringСhunks[1] != nullptr)) ? neighbouringСhunks[1]->voxels[((Y) * CHUNK_D + (Z)) * CHUNK_W + ((X) + CHUNK_W)].id : 0)
#define R4(X,Y,Z) (((X) == CHUNK_W && YZ_IS_IN(Y, Z) && (neighbouringСhunks[0] != nullptr)) ? neighbouringСhunks[0]->voxels[((Y) * CHUNK_D + (Z)) * CHUNK_W + ((X) - CHUNK_W)].id : 0)
#define G4(X,Y,Z) (((Z) == -1 && XY_IS_IN(X, Y) && (neighbouringСhunks[3] != nullptr)) ? neighbouringСhunks[3]->voxels[((Y) * CHUNK_D + ((Z) + CHUNK_D)) * CHUNK_W + (X)].id : 0)
#define H4(X,Y,Z) (((Z) == CHUNK_D && XY_IS_IN(X, Y) && (neighbouringСhunks[2] != nullptr)) ? neighbouringСhunks[2]->voxels[((Y) * CHUNK_D + ((Z) - CHUNK_D)) * CHUNK_W + (X)].id : 0)

#define Z4(X,Y,Z) (((X) == -1 && (Z) == -1 && Y_IS_IN(Y) && (neighbouringСhunks[4] != nullptr)) ? neighbouringСhunks[4]->voxels[((Y) * CHUNK_D + ((Z) + CHUNK_D)) * CHUNK_W + ((X) + CHUNK_W)].id : 0)
#define X4(X,Y,Z) (((X) == -1 && (Z) == CHUNK_D && Y_IS_IN(Y) && (neighbouringСhunks[6] != nullptr)) ? neighbouringСhunks[6]->voxels[((Y) * CHUNK_D + ((Z) - CHUNK_D)) * CHUNK_W + ((X) + CHUNK_W)].id : 0)
#define C4(X,Y,Z) (((Z) == -1 && (X) == CHUNK_W && Y_IS_IN(Y) && (neighbouringСhunks[5] != nullptr)) ? neighbouringСhunks[5]->voxels[((Y) * CHUNK_D + ((Z) + CHUNK_D)) * CHUNK_W + ((X) - CHUNK_W)].id : 0)
#define V4(X,Y,Z) (((Z) == CHUNK_D && (X) == CHUNK_W && Y_IS_IN(Y) && (neighbouringСhunks[7] != nullptr)) ? neighbouringСhunks[7]->voxels[((Y) * CHUNK_D + ((Z) - CHUNK_D)) * CHUNK_W + ((X) - CHUNK_W)].id : 0)

#define VERTEX(L, U, V, X, Y, Z)    chunk.vertices[verticesIndex + 0] = (L);\
                                    chunk.vertices[verticesIndex + 1] = (U);\
                                    chunk.vertices[verticesIndex + 2] = (V);\
                                    chunk.vertices[verticesIndex + 3] = (X);\
                                    chunk.vertices[verticesIndex + 4] = (Y);\
                                    chunk.vertices[verticesIndex + 5] = (Z);\
                                    verticesIndex += VERTEX_SIZE;

void ChunkRenderer::render(Chunk& chunk, const Chunk** neighbouringСhunks) {
    unsigned verticesIndex = 0;
    unsigned indexesIndex = 0;

    unsigned index = 0;
    unsigned int id;

    float brithness;
    float uvsize = 0.0625f;
    float u;
    float v;

    float aoFactor = 0.15f;
    float a, b, c, d, e, f, g, h;
    a = b = c = d = e = f = g = h = 0.0f;

    for (int y = 0; y < CHUNK_H; y++) {
        for (int z = 0; z < CHUNK_D; z++) {
            for (int x = 0; x < CHUNK_W; x++) {
                id = chunk.voxels[(y * CHUNK_D + z) * CHUNK_W + x].id;

                if (!id)
                    continue;

                u = (id % 16) * uvsize;
                v = (id / 16) * uvsize;

                if (!IS_BLOCKED(x + 1, y, z) && !R4(x + 1, y, z)) {

                    brithness = 0.95f;

                    a = (IS_BLOCKED(x + 1, y + 1, z) || R4(x + 1, y + 1, z)) * aoFactor;
                    b = (IS_BLOCKED(x + 1, y, z + 1) || R4(x + 1, y, z + 1) || H4(x + 1, y, z + 1) || V4(x + 1, y, z + 1)) * aoFactor;
                    c = (IS_BLOCKED(x + 1, y - 1, z) || R4(x + 1, y - 1, z)) * aoFactor;
                    d = (IS_BLOCKED(x + 1, y, z - 1) || R4(x + 1, y, z - 1) || G4(x + 1, y, z + 1) || C4(x + 1, y, z - 1)) * aoFactor;

                    e = (IS_BLOCKED(x + 1, y - 1, z - 1) || R4(x + 1, y - 1, z - 1) || G4(x + 1, y - 1, z - 1) || C4(x + 1, y - 1, z - 1)) * aoFactor;
                    f = (IS_BLOCKED(x + 1, y - 1, z + 1) || R4(x + 1, y - 1, z + 1) || H4(x + 1, y - 1, z + 1) || V4(x + 1, y - 1, z + 1)) * aoFactor;
                    g = (IS_BLOCKED(x + 1, y + 1, z + 1) || R4(x + 1, y + 1, z + 1) || H4(x + 1, y + 1, z + 1) || V4(x + 1, y + 1, z + 1)) * aoFactor;
                    h = (IS_BLOCKED(x + 1, y + 1, z - 1) || R4(x + 1, y + 1, z - 1) || G4(x + 1, y + 1, z - 1) || C4(x + 1, y + 1, z - 1)) * aoFactor;

                    VERTEX(brithness * (1.0f - a - b - g), u,          v,          x + 0.5f, y + 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - a - d - h), u + uvsize, v,          x + 0.5f, y + 0.5f, z - 0.5f);
                    VERTEX(brithness * (1.0f - b - c - f), u,          v + uvsize, x + 0.5f, y - 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - c - d - e), u + uvsize, v + uvsize, x + 0.5f, y - 0.5f, z - 0.5f);
                    chunk.currentVerticesCount += 4;

                    chunk.indexes[indexesIndex + 0] = index + 0;
                    chunk.indexes[indexesIndex + 1] = index + 2;
                    chunk.indexes[indexesIndex + 2] = index + 1;
                    chunk.indexes[indexesIndex + 3] = index + 1;
                    chunk.indexes[indexesIndex + 4] = index + 2;
                    chunk.indexes[indexesIndex + 5] = index + 3;
                    chunk.currentIndexesCount += 6;
                    indexesIndex += 6;

                    index += 4;
                }

                //u = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[1] % 16) * uvsize;
                //v = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[1] / 16) * uvsize;

                if (!IS_BLOCKED(x - 1, y, z) && !Q4(x - 1, y, z)) {

                    brithness = 0.85f;

                    a = (IS_BLOCKED(x - 1, y + 1, z) || Q4(x - 1, y + 1, z)) * aoFactor;
                    b = (IS_BLOCKED(x - 1, y, z + 1) || Q4(x - 1, y, z + 1) || H4(x - 1, y, z + 1) || X4(x - 1, y, z + 1)) * aoFactor;
                    c = (IS_BLOCKED(x - 1, y - 1, z) || Q4(x - 1, y - 1, z)) * aoFactor;
                    d = (IS_BLOCKED(x - 1, y, z - 1) || Q4(x - 1, y, z - 1) || G4(x - 1, y, z - 1) || Z4(x - 1, y, z - 1)) * aoFactor;

                    e = (IS_BLOCKED(x - 1, y - 1, z - 1) || Q4(x - 1, y - 1, z - 1) || G4(x - 1, y - 1, z - 1) || Z4(x - 1, y - 1, z - 1)) * aoFactor;
                    f = (IS_BLOCKED(x - 1, y - 1, z + 1) || Q4(x - 1, y - 1, z + 1) || H4(x - 1, y - 1, z + 1) || X4(x - 1, y - 1, z + 1)) * aoFactor;
                    g = (IS_BLOCKED(x - 1, y + 1, z + 1) || Q4(x - 1, y + 1, z + 1) || H4(x - 1, y + 1, z + 1) || X4(x - 1, y + 1, z + 1)) * aoFactor;
                    h = (IS_BLOCKED(x - 1, y + 1, z - 1) || Q4(x - 1, y + 1, z - 1) || G4(x - 1, y + 1, z - 1) || Z4(x - 1, y + 1, z - 1)) * aoFactor;

                    VERTEX(brithness * (1.0f - a - b - g), u + uvsize, v,          x - 0.5f, y + 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - a - d - h), u,          v,          x - 0.5f, y + 0.5f, z - 0.5f);
                    VERTEX(brithness * (1.0f - b - c - f), u + uvsize, v + uvsize, x - 0.5f, y - 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - c - d - e), u,          v + uvsize, x - 0.5f, y - 0.5f, z - 0.5f);
                    chunk.currentVerticesCount += 4;

                    chunk.indexes[indexesIndex + 0] = index + 0;
                    chunk.indexes[indexesIndex + 1] = index + 1;
                    chunk.indexes[indexesIndex + 2] = index + 2;
                    chunk.indexes[indexesIndex + 3] = index + 2;
                    chunk.indexes[indexesIndex + 4] = index + 1;
                    chunk.indexes[indexesIndex + 5] = index + 3;
                    indexesIndex += 6;
                    chunk.currentIndexesCount += 6;

                    index += 4;
                }

                //u = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[2] % 16) * uvsize;
                //v = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[2] / 16) * uvsize;

                if (!IS_BLOCKED(x, y + 1, z)) {

                    brithness = 1.0f;

                    a = (IS_BLOCKED(x + 1, y + 1, z) || R4(x + 1, y + 1, z)) * aoFactor;
                    b = (IS_BLOCKED(x, y + 1, z + 1) || H4(x, y + 1, z + 1)) * aoFactor;
                    c = (IS_BLOCKED(x - 1, y + 1, z) || Q4(x - 1, y + 1, z)) * aoFactor;
                    d = (IS_BLOCKED(x, y + 1, z - 1) || G4(x, y + 1, z - 1)) * aoFactor;

                    e = (IS_BLOCKED(x - 1, y + 1, z - 1) || Q4(x - 1, y + 1, z - 1) || G4(x - 1, y + 1, z - 1) || Z4(x - 1, y + 1, z - 1)) * aoFactor;
                    f = (IS_BLOCKED(x - 1, y + 1, z + 1) || Q4(x - 1, y + 1, z + 1) || H4(x - 1, y + 1, z + 1) || X4(x - 1, y + 1, z + 1)) * aoFactor;
                    g = (IS_BLOCKED(x + 1, y + 1, z + 1) || R4(x + 1, y + 1, z + 1) || H4(x + 1, y + 1, z + 1) || V4(x + 1, y + 1, z + 1)) * aoFactor;
                    h = (IS_BLOCKED(x + 1, y + 1, z - 1) || R4(x + 1, y + 1, z - 1) || G4(x + 1, y + 1, z - 1) || C4(x + 1, y + 1, z - 1)) * aoFactor;

                    VERTEX(brithness * (1.0f - a - b - g), u + uvsize, v + uvsize, x + 0.5f, y + 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - a - d - h), u + uvsize, v,          x + 0.5f, y + 0.5f, z - 0.5f);
                    VERTEX(brithness * (1.0f - b - c - f), u,          v + uvsize, x - 0.5f, y + 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - c - d - e), u,          v,          x - 0.5f, y + 0.5f, z - 0.5f);
                    chunk.currentVerticesCount += 4;

                    chunk.indexes[indexesIndex + 0] = index + 0;
                    chunk.indexes[indexesIndex + 1] = index + 1;
                    chunk.indexes[indexesIndex + 2] = index + 2;
                    chunk.indexes[indexesIndex + 3] = index + 2;
                    chunk.indexes[indexesIndex + 4] = index + 1;
                    chunk.indexes[indexesIndex + 5] = index + 3;
                    indexesIndex += 6;
                    chunk.currentIndexesCount += 6;

                    index += 4;
                }

                //u = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[3] % 16) * uvsize;
                //v = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[3] / 16) * uvsize;

                if (!IS_BLOCKED(x, y - 1, z)) {

                    brithness = 0.75f;

                    a = (IS_BLOCKED(x + 1, y - 1, z) || R4(x + 1, y - 1, z)) * aoFactor;
                    b = (IS_BLOCKED(x, y - 1, z + 1) || H4(x, y - 1, z + 1)) * aoFactor;
                    c = (IS_BLOCKED(x - 1, y - 1, z) || Q4(x - 1, y - 1, z)) * aoFactor;
                    d = (IS_BLOCKED(x, y - 1, z - 1) || G4(x, y - 1, z - 1)) * aoFactor;

                    e = (IS_BLOCKED(x - 1, y - 1, z - 1) || Q4(x - 1, y - 1, z - 1) || G4(x - 1, y - 1, z - 1) || Z4(x - 1, y - 1, z - 1)) * aoFactor;
                    f = (IS_BLOCKED(x - 1, y - 1, z + 1) || Q4(x - 1, y - 1, z + 1) || H4(x - 1, y - 1, z + 1) || X4(x - 1, y - 1, z + 1)) * aoFactor;
                    g = (IS_BLOCKED(x + 1, y - 1, z + 1) || R4(x + 1, y - 1, z + 1) || H4(x + 1, y - 1, z + 1) || V4(x + 1, y - 1, z + 1)) * aoFactor;
                    h = (IS_BLOCKED(x + 1, y - 1, z - 1) || R4(x + 1, y - 1, z - 1) || G4(x + 1, y - 1, z - 1) || C4(x + 1, y - 1, z - 1)) * aoFactor;

                    VERTEX(brithness * (1.0f - a - b - g), u + uvsize, v,          x + 0.5f, y - 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - a - d - h), u + uvsize, v + uvsize, x + 0.5f, y - 0.5f, z - 0.5f);
                    VERTEX(brithness * (1.0f - b - c - f), u,          v,          x - 0.5f, y - 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - c - d - e), u,          v + uvsize, x - 0.5f, y - 0.5f, z - 0.5f);
                    chunk.currentVerticesCount += 4;

                    chunk.indexes[indexesIndex + 0] = index + 0;
                    chunk.indexes[indexesIndex + 1] = index + 2;
                    chunk.indexes[indexesIndex + 2] = index + 1;
                    chunk.indexes[indexesIndex + 3] = index + 1;
                    chunk.indexes[indexesIndex + 4] = index + 2;
                    chunk.indexes[indexesIndex + 5] = index + 3;
                    indexesIndex += 6;
                    chunk.currentIndexesCount += 6;

                    index += 4;
                }

                //u = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[4] % 16) * uvsize;
                //v = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[4] / 16) * uvsize;

                if (!IS_BLOCKED(x, y, z + 1) && !H4(x, y, z + 1)) {

                    brithness = 0.9f;

                    a = (IS_BLOCKED(x, y + 1, z + 1) || H4(x, y + 1, z + 1)) * aoFactor;
                    b = (IS_BLOCKED(x + 1, y, z + 1) || H4(x + 1, y, z + 1) || R4(x + 1, y, z + 1) || V4(x + 1, y, z + 1)) * aoFactor;
                    c = (IS_BLOCKED(x, y - 1, z + 1) || H4(x, y - 1, z + 1)) * aoFactor;
                    d = (IS_BLOCKED(x - 1, y, z + 1) || H4(x - 1, y, z + 1) || Q4(x - 1, y, z + 1) || X4(x - 1, y, z + 1)) * aoFactor;

                    e = (IS_BLOCKED(x - 1, y - 1, z + 1) || H4(x - 1, y - 1, z + 1) || Q4(x - 1, y - 1, z + 1) || X4(x - 1, y - 1, z + 1)) * aoFactor;
                    f = (IS_BLOCKED(x + 1, y - 1, z + 1) || H4(x + 1, y - 1, z + 1) || R4(x + 1, y - 1, z + 1) || V4(x + 1, y - 1, z + 1)) * aoFactor;
                    g = (IS_BLOCKED(x + 1, y + 1, z + 1) || H4(x + 1, y + 1, z + 1) || R4(x + 1, y + 1, z + 1) || V4(x + 1, y + 1, z + 1)) * aoFactor;
                    h = (IS_BLOCKED(x - 1, y + 1, z + 1) || H4(x - 1, y + 1, z + 1) || Q4(x - 1, y + 1, z + 1) || X4(x - 1, y + 1, z + 1)) * aoFactor;

                    VERTEX(brithness * (1.0f - a - b - g), u + uvsize, v,          x + 0.5f, y + 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - b - c - f), u + uvsize, v + uvsize, x + 0.5f, y - 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - a - d - h), u,          v,          x - 0.5f, y + 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - c - d - e), u,          v + uvsize, x - 0.5f, y - 0.5f, z + 0.5f);
                    chunk.currentVerticesCount += 4;

                    chunk.indexes[indexesIndex + 0] = index + 0;
                    chunk.indexes[indexesIndex + 1] = index + 2;
                    chunk.indexes[indexesIndex + 2] = index + 1;
                    chunk.indexes[indexesIndex + 3] = index + 1;
                    chunk.indexes[indexesIndex + 4] = index + 2;
                    chunk.indexes[indexesIndex + 5] = index + 3;
                    indexesIndex += 6;
                    chunk.currentIndexesCount += 6;

                    index += 4;
                }

                //u = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[5] % 16) * uvsize;
                //v = (chunk.voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x].VoxelFaces[5] / 16) * uvsize;

                if (!IS_BLOCKED(x, y, z - 1) && !G4(x, y, z - 1)) {

                    brithness = 0.8f;

                    a = (IS_BLOCKED(x, y + 1, z - 1) || G4(x, y + 1, z - 1)) * aoFactor;
                    b = (IS_BLOCKED(x + 1, y, z - 1) || G4(x + 1, y, z - 1) || R4(x + 1, y, z - 1) || C4(x + 1, y, z - 1)) * aoFactor;
                    c = (IS_BLOCKED(x, y - 1, z - 1) || G4(x, y - 1, z - 1)) * aoFactor;
                    d = (IS_BLOCKED(x - 1, y, z - 1) || G4(x - 1, y, z - 1) || Q4(x - 1, y, z - 1) || Z4(x - 1, y, z - 1)) * aoFactor;

                    e = (IS_BLOCKED(x - 1, y - 1, z - 1) || G4(x - 1, y - 1, z - 1) || Q4(x - 1, y - 1, z - 1) || Z4(x - 1, y - 1, z - 1)) * aoFactor;
                    f = (IS_BLOCKED(x + 1, y - 1, z - 1) || G4(x + 1, y - 1, z - 1) || R4(x + 1, y - 1, z - 1) || C4(x + 1, y - 1, z - 1)) * aoFactor;
                    g = (IS_BLOCKED(x + 1, y + 1, z - 1) || G4(x + 1, y + 1, z - 1) || R4(x + 1, y + 1, z - 1) || C4(x + 1, y + 1, z - 1)) * aoFactor;
                    h = (IS_BLOCKED(x - 1, y + 1, z - 1) || G4(x - 1, y + 1, z - 1) || Q4(x - 1, y + 1, z - 1) || Z4(x - 1, y + 1, z - 1)) * aoFactor;

                    VERTEX(brithness * (1.0f - a - b - g), u,          v,          x + 0.5f, y + 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - b - c - f), u,          v + uvsize, x + 0.5f, y - 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - a - d - h), u + uvsize, v,          x - 0.5f, y + 0.5f, z + 0.5f);
                    VERTEX(brithness * (1.0f - c - d - e), u + uvsize, v + uvsize, x - 0.5f, y - 0.5f, z + 0.5f);
                    chunk.currentVerticesCount += 4;

                    chunk.indexes[indexesIndex + 0] = index + 0;
                    chunk.indexes[indexesIndex + 1] = index + 1;
                    chunk.indexes[indexesIndex + 2] = index + 2;
                    chunk.indexes[indexesIndex + 3] = index + 2;
                    chunk.indexes[indexesIndex + 4] = index + 1;
                    chunk.indexes[indexesIndex + 5] = index + 3;
                    indexesIndex += 6;
                    chunk.currentIndexesCount += 6;

                    index += 4;
                }

            }
        }
    }
}