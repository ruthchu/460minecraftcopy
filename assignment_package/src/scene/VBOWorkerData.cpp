#include "VBOWorkerData.h"

SharedVBODataCollection::SharedVBODataCollection()
    : VBOchunks(std::vector<Chunk*>()), mu(std::mutex())
{}

SharedVBODataCollection::~SharedVBODataCollection() {}

SharedVBODataCollection::SharedVBODataCollection(const SharedVBODataCollection &collection)
    : VBOchunks(collection.VBOchunks), mu()
{}

void SharedVBODataCollection::addChunk(Chunk* data) {
    //std::lock_guard<std::mutex> lock(this->mu);
    mu.lock();
    this->VBOchunks.push_back(data);
    mu.unlock();
}

void SharedVBODataCollection::clearChunkData() {
    //mu.lock();
    this->VBOchunks.clear();
    //mu.unlock();
}

bool SharedVBODataCollection::isEmpty() {
    return this->VBOchunks.empty();
}

std::vector<Chunk*> SharedVBODataCollection::getVectorData() {
    return this->VBOchunks;
}
