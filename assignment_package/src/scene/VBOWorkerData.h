#ifndef VBOWORKERDATA_H
#define VBOWORKERDATA_H

#include "chunk.h"

class SharedVBODataCollection {
public:
    struct VBOData {
        std::vector<float> vertexData;
        std::vector<GLuint> idx;
        Chunk* chunk;
        VBOData(Chunk* chunk);
    };
    std::vector<VBOData> VBOchunks;
};

#endif // VBOWORKERDATA_H
