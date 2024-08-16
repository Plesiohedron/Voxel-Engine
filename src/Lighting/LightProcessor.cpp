#include "LightProcessor.h"
#include "Lightmap.h"

LightProcessor::LightProcessor(const ChunkStorage* chunk_storage, int channel) : chunk_storage_(chunk_storage), channel_(channel) { }

void LightProcessor::Add(int x, int y, int z, uint8_t emission) {
    if (emission <= 1) {
        return;
    }

    Chunk* chunk = chunk_storage_->GetChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        return;
    }

    add_queue_.push({x, y, z, emission});

    chunk->is_modified = true;
    chunk->lightmap_->Set(x - chunk->global_coordinates.x * Chunk::WIDTH,
                         y - chunk->global_coordinates.y * Chunk::HEIGHT,
                         z - chunk->global_coordinates.z * Chunk::DEPTH,
                         channel_, emission);
}

void LightProcessor::Add(int x, int y, int z) {
    Add(x, y, z, chunk_storage_->GetLight(x, y, z, channel_));
}

void LightProcessor::Remove(int x, int y, int z) {
    Chunk* chunk = chunk_storage_->GetChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        return;
    }

    uint8_t emission = chunk->lightmap_->Get(x - chunk->global_coordinates.x * Chunk::WIDTH,
                                            y - chunk->global_coordinates.y * Chunk::HEIGHT,
                                            z - chunk->global_coordinates.z * Chunk::DEPTH,
                                            channel_);
    if (emission == 0) {
        return;
    }

    remove_queue_.push({x, y, z, emission});

    chunk->lightmap_->Set(x - chunk->global_coordinates.x * Chunk::WIDTH,
                         y - chunk->global_coordinates.y * Chunk::HEIGHT,
                         z - chunk->global_coordinates.z * Chunk::DEPTH,
                         channel_, 0);
}

void LightProcessor::Process() {
    const int coords[] = {0, 0, 1,
                          0, 0, -1,
                          0, 1, 0,
                          0, -1, 0,
                          1, 0, 0,
                          -1, 0, 0};

    while (!remove_queue_.empty()) {
        light entry = remove_queue_.front();
        remove_queue_.pop();

        for (int i = 0; i < 6; ++i) {
            int x = entry.x + coords[i * 3 + 0];
            int y = entry.y + coords[i * 3 + 1];
            int z = entry.z + coords[i * 3 + 2];
            Chunk* chunk = chunk_storage_->GetChunkByVoxel(x, y, z);
            if (chunk) {
                uint8_t emission = chunk_storage_->GetLight(x, y, z, channel_);
                if (emission != 0 && emission == entry.emission - 1) {
                    remove_queue_.push({x, y, z, emission});
                    chunk->is_modified = true;
                    chunk->lightmap_->Set(x - chunk->global_coordinates.x * Chunk::WIDTH,
                                         y - chunk->global_coordinates.y * Chunk::HEIGHT,
                                         z - chunk->global_coordinates.z * Chunk::DEPTH,
                                         channel_, 0);
                } else if (emission >= entry.emission) {
                    add_queue_.push({x, y, z, emission});
                }
            }
        }
    }

    while (!add_queue_.empty()) {
        light entry = add_queue_.front();
        add_queue_.pop();

        if (entry.emission <= 1) {
            continue;
        }

        for (int i = 0; i < 6; ++i) {
            int x = entry.x + coords[i * 3 + 0];
            int y = entry.y + coords[i * 3 + 1];
            int z = entry.z + coords[i * 3 + 2];
            Chunk* chunk = chunk_storage_->GetChunkByVoxel(x, y, z);
            if (chunk) {
                uint8_t emission = chunk_storage_->GetLight(x, y, z, channel_);
                Voxel* voxel = chunk_storage_->GetVoxel(x, y, z);
                if (voxel->id == 0 && emission + 2 <= entry.emission) {
                    chunk->lightmap_->Set(x - chunk->global_coordinates.x * Chunk::WIDTH,
                                         y - chunk->global_coordinates.y * Chunk::HEIGHT,
                                         z - chunk->global_coordinates.z * Chunk::DEPTH,
                                         channel_, entry.emission - 1);
                    chunk->is_modified = true;
                    add_queue_.push({x, y, z, static_cast<uint8_t>(entry.emission - 1)});
                }
            }
        }
    }
}
