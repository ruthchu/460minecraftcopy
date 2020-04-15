#include "BlockTypeData.h"

SharedBlockTypeCollection::SharedBlockTypeCollection()
    : filledChunks(), mu()
{}

SharedBlockTypeCollection::~SharedBlockTypeCollection() {}

SharedBlockTypeCollection::SharedBlockTypeCollection(const SharedBlockTypeCollection &collection)
    : filledChunks(collection.filledChunks), mu()
{}

void SharedBlockTypeCollection::addChunk(Chunk* chunk) {
    std::lock_guard<std::mutex> lock(this->mu);
    START_PRINT std::this_thread::get_id() END_PRINT;
    this->filledChunks.push_back(chunk);
}
