#include "Chunk.h"

#include <math.h>
#include <iostream>

Chunk::Chunk() {
    voxels = new Voxel[CHUNK_VOL]();

    uint8_t id;
    for (int y = 0; y < CHUNK_H; y++) {
        for (int z = 0; z < CHUNK_D; z++) {
            for (int x = 0; x < CHUNK_W; x++) {
                id = y <= 10;
                if (y <= 2)
                    id = 4;

                // voxels[y][z][x] := voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]
                voxels[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
            }
        }
    }

    vertices = new GLushort[6 * ATTRIBUTES_COUNT * (CHUNK_H + 1) * (CHUNK_D + 1) * (CHUNK_W + 1)]();
    indexes = new GLushort[6 * (CHUNK_H + 1) * (CHUNK_D + 1) * (CHUNK_W + 1)]();
}

Chunk::~Chunk() {
    delete[] voxels;
    delete[] vertices;
    delete[] indexes;
}
