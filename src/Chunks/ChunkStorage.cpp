#include "ChunkStorage.h"

glm::ivec3 ChunkStorage::storage_sizes;

ChunkStorage::ChunkStorage(const int radius, const glm::ivec3& center)
    : model(1.0f), rendering_radius(radius), rendering_center(center) {

    storage_sizes = {2 * radius - 1, CHUNKS_COUNT_IN_HEIGHT, 2 * radius - 1};
    chunks.resize(storage_sizes.x * storage_sizes.y * storage_sizes.z);

    for (int y = 0; y < storage_sizes.y; ++y) {
        for (int z = 0; z < storage_sizes.z; ++z) {
            for (int x = 0; x < storage_sizes.z; ++x) {
                chunks[(y * storage_sizes.z + z) * storage_sizes.x + x] = new Chunk({x, y, z});
            }
        }
    }
}

ChunkStorage::~ChunkStorage() {
    for (Chunk* chunk : chunks) {
        delete chunk;
    }
}
