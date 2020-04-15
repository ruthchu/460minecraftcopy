#ifndef BLOCKTYPEDATA_H
#define BLOCKTYPEDATA_H

#include "chunk.h"
#include <mutex>
#include <thread>
#include "smartpointerhelp.h"
#include <iostream>
#define BlockData SharedBlockTypeCollection

class SharedBlockTypeCollection {
public:
    std::vector<Chunk*> filledChunks;
    std::mutex mu;

    SharedBlockTypeCollection();

    void addChunk(Chunk* chunk);

    ~SharedBlockTypeCollection();
    SharedBlockTypeCollection(const SharedBlockTypeCollection &collection);
};

#endif // BLOCKTYPEDATA_H
