#include "Chunk.h"

#include <math.h>

Chunk::Chunk() {
    voxels = new Voxel[VOLUME];

    for (int y = 0; y < HEIGHT; y++) {
        for (int z = 0; z < DEPTH; z++) {
            for (int x = 0; x < WIDTH; x++) {
                int id = y <= (sin(x * 0.3f) * 0.5f + 0.5f) * 10;
                if (y <= 2)
                    id = 2;

                // voxels[y][z][x] := voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]
                voxels[(y * DEPTH + z) * WIDTH + x].id = id;
            }
        }
    }

    vertices = new float[ATTRIBUTES_COUNT * ((HEIGHT + 1) * (DEPTH + 1) * (WIDTH + 1))];
    indexes = new unsigned[(HEIGHT + 1) * (DEPTH + 1) * (WIDTH + 1)];
}

Chunk::~Chunk() {
    delete[] vertices;
    delete[] indexes;
}
