#include "VBOWorkerData.h"

SharedVBODataCollection::SharedVBODataCollection()
    : VBOchunks(std::vector<VBOData>()), mu(std::mutex())
{}

SharedVBODataCollection::~SharedVBODataCollection() {}

SharedVBODataCollection::SharedVBODataCollection(const SharedVBODataCollection &collection)
    : VBOchunks(collection.VBOchunks), mu()
{}

void SharedVBODataCollection::addChunk(VBOData data) {
    std::lock_guard<std::mutex> lock(this->mu);
    this->VBOchunks.push_back(data);
}

void SharedVBODataCollection::clearChunkData() {
    this->VBOchunks.clear();
}

bool SharedVBODataCollection::isEmpty() {
    return this->VBOchunks.empty();
}

std::vector<VBOData> SharedVBODataCollection::getVectorData() {
    return this->VBOchunks;
}
