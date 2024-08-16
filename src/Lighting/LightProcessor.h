#pragma once

#include "../Chunks/ChunkStorage.h"

#include <queue>

struct light {
    int x;
    int y;
    int z;
    uint8_t emission;
};

class LightProcessor {
    friend class ChunkStorage;

private:
    std::queue<light> add_queue_;
    std::queue<light> remove_queue_;

    const ChunkStorage* chunk_storage_;
    int channel_;

private:
    LightProcessor(const ChunkStorage* chunk_storage, int channel);

    void Add(int x, int y, int z);
    void Add(int x, int y, int z, uint8_t emission);
    void Remove(int x, int y, int z);
    void Process();
};
