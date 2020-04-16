#ifndef VBOWORKERDATA_H
#define VBOWORKERDATA_H

#include "chunk.h"
#include <mutex>

class SharedVBODataCollection {
public:
    struct VBOData {
        std::vector<float> vertexData;
        std::vector<GLuint> idx;
        Chunk* chunk;
        VBOData(Chunk* chunk);
    };

    std::vector<VBOData> VBOchunks;
    std::mutex mu;

    SharedVBODataCollection();

    void addChunk(VBOData data);
    void clearChunkData();
    bool isEmpty();

    // return copy of vector
    std::vector<VBOData> getVectorData();

    ~SharedVBODataCollection();
    SharedVBODataCollection(const SharedVBODataCollection &collection);
};

#endif // VBOWORKERDATA_H
