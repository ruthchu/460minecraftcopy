#ifndef BLOCKTYPEDATA_H
#define BLOCKTYPEDATA_H

#include "chunk.h"
#include <mutex>
#include <thread>
#include "smartpointerhelp.h"
#include <iostream>
#include <vector>
#define BlockData SharedBlockTypeCollection

class SharedBlockTypeCollection {
public:
    std::vector<Chunk*> filledChunks;
    std::mutex mu;

    SharedBlockTypeCollection();

    void addChunk(Chunk* chunk);
    void clearChunkData();
    bool isEmpty();

    // return copy of vector
    std::vector<Chunk*> getVectorData();

    ~SharedBlockTypeCollection();
    SharedBlockTypeCollection(const SharedBlockTypeCollection &collection);
};

#endif // BLOCKTYPEDATA_H
