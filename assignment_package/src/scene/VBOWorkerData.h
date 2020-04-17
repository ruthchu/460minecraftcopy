#ifndef VBOWORKERDATA_H
#define VBOWORKERDATA_H

#include "chunk.h"
#include <mutex>
#define VBOCollection SharedVBODataCollection

struct VBOData {
    std::vector<GLuint> idx;
    std::vector<float> vertexData;
    Chunk* c;
    VBOData(Chunk* chunk) {
        c = chunk;
    }
};

class SharedVBODataCollection {
public:
    std::vector<Chunk*> VBOchunks;
    std::mutex mu;

    SharedVBODataCollection();

    void addChunk(Chunk* data);
    void clearChunkData();
    bool isEmpty();

    // return copy of vector
    std::vector<Chunk*> getVectorData();

    ~SharedVBODataCollection();
    SharedVBODataCollection(const SharedVBODataCollection &collection);
};

#endif // VBOWORKERDATA_H
