#include "ChunkRenderer.h"

#include <iostream>

#define IS_IN(X, Y, Z) ((X) >= 0 && (X) < CHUNK_W && (Y) >= 0 && (Y) < CHUNK_H && (Z) >= 0 && (Z) < CHUNK_D)
#define VOXEL(X, Y, Z) (chunk.voxels[((Y) * CHUNK_D + (Z)) * CHUNK_W + (X)])
#define IS_BLOCKED(X, Y, Z) ((IS_IN(X, Y, Z)) && VOXEL(X, Y, Z).id)

#define VERTEX(idx, r, g, b, s, u, v, x, y, z)\
    chunk.vertices[idx + 0] = ((x << 10) | (y << 5) | z);\
    chunk.vertices[idx + 1] = ((u << 5) | v);\
    chunk.vertices[idx + 2] = ((r << 12) | (g << 8) | (b << 4) | s);\
    idx += VERTEX_SIZE;

#define INDEX(IDX, VALUE)\
    chunk.indexes[IDX + 0] = VALUE + 0;\
    chunk.indexes[IDX + 1] = VALUE + 1;\
    chunk.indexes[IDX + 2] = VALUE + 2;\
    chunk.indexes[IDX + 3] = VALUE + 2;\
    chunk.indexes[IDX + 4] = VALUE + 1;\
    chunk.indexes[IDX + 5] = VALUE + 3;\
    IDX += INDEXES_COUNT_PER_SQUARE;\
    VALUE += VERTICES_COUNT_PER_SQUARE;

void ChunkRenderer::render(Chunk& chunk) {
    GLushort index = 0;
    uint8_t id;

    GLushort brithness;
    GLushort UVx;
    GLushort UVy;

    for (GLushort y = 0; y < CHUNK_H; y++) {
        for (GLushort z = 0; z < CHUNK_D; z++) {
            for (GLushort x = 0; x < CHUNK_W; x++) {
                id = chunk.voxels[(y * CHUNK_D + z) * CHUNK_W + x].id;

                if (!id)
                    continue;

                UVx = ((id - 1) % 16);
                UVy = ((id - 1) / 16);

                if (!IS_BLOCKED(x - 1, y, z)) {
                    brithness = 13;

                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x, y + 1, z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x, y + 1, z);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x, y,     z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x, y,     z);

                    INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x + 1, y, z)) {
                    brithness = 14;

                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x + 1, y + 1, z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x + 1, y,     z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x + 1, y + 1, z);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x + 1, y,     z);

                    INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x, y - 1, z)) {
                    brithness = 11;

                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x + 1, y, z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x,     y, z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x + 1, y, z);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x,     y, z);

                    INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x, y + 1, z)) {
                    brithness = 15;

                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x + 1, y + 1, z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x + 1, y + 1, z);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x,     y + 1, z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x,     y + 1, z);

                    INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x, y, z - 1)) {
                    brithness = 12;

                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x + 1, y + 1, z);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x + 1, y,     z);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x,     y + 1, z);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x,     y,     z);

                    INDEX(chunk.currentIndexesCount, index);
                }

                if (!IS_BLOCKED(x, y, z + 1)) {
                    brithness = 14;

                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy,     x + 1, y + 1, z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy,     x,     y + 1, z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx + 1, UVy + 1, x + 1, y,     z + 1);
                    VERTEX(chunk.currentVerticesCount, brithness, brithness, brithness, 15, UVx,     UVy + 1, x,     y,     z + 1);

                    INDEX(chunk.currentIndexesCount, index);
                }
            }
        }
    }
}