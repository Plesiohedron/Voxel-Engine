#include "Chunk.h"

#include <math.h>
#include <iostream>

Chunk::Chunk() : meshData(
                          3, std::vector<std::vector<std::vector<std::vector<uint32_t>>>>(
                              2, std::vector<std::vector<std::vector<uint32_t>>>(
                                  16, std::vector<std::vector<uint32_t>>(
                                      16, std::vector<uint32_t>(16, 0)
                                  )
                              )
                          )
                 ) {

    voxels = new Voxel[CHUNK_VOL]();

    uint8_t id;
    for (int y = 0; y < CHUNK_H; ++y) {
        for (int z = 0; z < CHUNK_D; ++z) {
            for (int x = 0; x < CHUNK_W; ++x) {
                id = y <= sin(0.1 * x) * 10;
                if (y <= 2)
                    id = 4;

                // voxels[y][z][x] := voxels[(y * CHUNK_DEPTH + z) * CHUNK_WIDTH + x]
                voxels[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
            }
        }
    }
}

Chunk::~Chunk() {
    delete[] voxels;
}
