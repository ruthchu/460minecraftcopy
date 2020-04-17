#include "BlockTypeData.h"

SharedBlockTypeCollection::SharedBlockTypeCollection()
    : filledChunks(std::vector<Chunk*>()), mu(std::mutex())
{}

SharedBlockTypeCollection::~SharedBlockTypeCollection() {}

SharedBlockTypeCollection::SharedBlockTypeCollection(const SharedBlockTypeCollection &collection)
    : filledChunks(collection.filledChunks), mu()
{}

void SharedBlockTypeCollection::addChunk(Chunk* chunk) {
    //std::lock_guard<std::mutex> lock(this->mu);
//    START_PRINT std::this_thread::get_id() END_PRINT;
    mu.lock();
    this->filledChunks.push_back(chunk);
    mu.unlock();
//    START_PRINT filledChunks.size() END_PRINT;
}

void SharedBlockTypeCollection::clearChunkData() {
    mu.lock();
    this->filledChunks.clear();
    mu.unlock();
}

bool SharedBlockTypeCollection::isEmpty() {
    return this->filledChunks.empty();
}

std::vector<Chunk*> SharedBlockTypeCollection::getVectorData() {
    return this->filledChunks;
}
