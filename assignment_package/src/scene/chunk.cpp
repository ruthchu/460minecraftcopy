#include "chunk.h"
#include <openglcontext.h>
#include <glm_includes.h>
#include <iostream>

Chunk::Chunk(OpenGLContext* context, int X, int Z)
    : idx(std::vector<GLuint>()), data(std::vector<glm::vec4>()), Drawable(context), X(X), Z(Z), m_blocks(),
      m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}

void Chunk::create()
{
    this->idx.clear();
    this->tIdx.clear();

    int indexCount = 0;
    int tIndexCount = 0;


    // Iterate over all blocks in chunk
    for (int i = 0; i < 16; i++) { // x
        for (int j = 0; j < 256; j++) { // y
            for (int k = 0; k < 16; k++) { // z
                // Block at current location
                BlockType t = getBlockAt(i, j, k);

                // World pos = chunk position + block local position
                glm::vec4 worldPos = glm::vec4(this->X, 0.f, this->Z, 0.f) + glm::vec4(i, j, k, 1.f);

                if (t == EMPTY) {
                    continue;
                } else if (t == DIRT || t == GRASS || t == STONE || t == SNOW ||
                           t == LAVA) { // Solid blocks
                    // Back face (face with LL vertex at worldPos)
                    BlockType blockBehind = getBlockAt(i, j, std::max(0, k - 1));
                    if (k == 0) {
                        if (m_neighbors.at(ZNEG) != nullptr) {
                            blockBehind = m_neighbors.at(ZNEG)->getBlockAt(i, j, 15);
                        }
                    }
                    if (blockBehind == EMPTY || blockBehind == WATER || blockBehind == ICE ||
                            (k == 0 && m_neighbors.at(ZNEG) == nullptr)) {
                        // Back face positions
                        // Back face normal is -k
                        glm::vec4 norm = glm::vec4(0.f, 0.f, -1.f, 0.f);
                        glm::vec4 uv = getUVs(t, ZNEG);
                        //UL
                        data.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        data.push_back(worldPos);
                        data.push_back(glm::vec4(0.f, 0.f, -1.f, 0.f));
                        data.push_back(uv);
                        //LR
                        data.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                        data.push_back(glm::vec4(0.f, 0.f, -1.f, 0.f));
                        data.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        data.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                        data.push_back(glm::vec4(0.f, 0.f, -1.f, 0.f));
                        data.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(idx, indexCount);
                        indexCount += 4;
                    }

                    // Front face
                    BlockType blockFront = getBlockAt(i, j, std::min(15, k + 1));
                    if (k == 15) {
                        if (m_neighbors.at(ZPOS) != nullptr) {
                            blockFront = m_neighbors.at(ZPOS)->getBlockAt(i, j, 0);
                        }
                    }
                    if (blockFront == EMPTY || blockFront == WATER || blockFront == ICE ||
                            (k == 15 && m_neighbors.at(ZPOS) == nullptr)) {
                        // Front face positions
                        // Front face normal is +k
                        glm::vec4 norm = glm::vec4(0.f, 0.f, 1.f, 0.f);
                        glm::vec4 uv = getUVs(t, ZPOS);
                        //UL
                        data.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        data.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv);
                        //LR
                        data.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        data.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(idx, indexCount);
                        indexCount += 4;
                    }

                    // Left face
                    BlockType blockLeft = getBlockAt(std::max(0, i - 1), j, k);
                    if (i == 0) {
                        if (m_neighbors.at(XNEG) != nullptr) {
                            blockLeft = m_neighbors.at(XNEG)->getBlockAt(15, j, k);
                        }
                    }
                    if (blockLeft == EMPTY || blockLeft == WATER || blockLeft == ICE ||
                            (i == 0 && m_neighbors.at(XNEG) == nullptr)) {
                        // Left face positions
                        // Left face normal is -i
                        glm::vec4 norm = glm::vec4(-1.f, 0.f, 0.f, 0.f);
                        glm::vec4 uv = getUVs(t, XNEG);
                        //UL
                        data.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        data.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv);
                        //LR
                        data.push_back(worldPos);
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        data.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(idx, indexCount);
                        indexCount += 4;
                    }

                    // Right face
                    BlockType blockRight = getBlockAt(std::min(15, i + 1), j, k);
                    if (i == 15) {
                        if (m_neighbors.at(XPOS) != nullptr) {
                            blockRight = m_neighbors.at(XPOS)->getBlockAt(0, j, k);
                        }
                    }
                    if (blockRight == EMPTY || blockRight == WATER || blockRight == ICE ||
                            (i == 15 && m_neighbors.at(XPOS) == nullptr)) {
                        // Right face positions
                        // Right face normal is +i
                        glm::vec4 norm = glm::vec4(1.f, 0.f, 0.f, 0.f);
                        glm::vec4 uv = getUVs(t, XPOS);
                        //UL
                        data.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        data.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv);
                        //LR
                        data.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        data.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(idx, indexCount);
                        indexCount += 4;
                    }

                    // Bottom face
                    BlockType blockBottom = getBlockAt(i, std::max(0, j - 1), k);
                    if (blockBottom == EMPTY || blockBottom == WATER ||
                            blockBottom == ICE || j == 0) {
                        // Bottom face positions
                        // Bottom face normal is -j
                        glm::vec4 norm = glm::vec4(0.f, -1.f, 0.f, 0.f);
                        glm::vec4 uv = getUVs(t, YNEG);
                        //UL
                        data.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        data.push_back(worldPos);
                        data.push_back(norm);
                        data.push_back(uv);
                        //LR
                        data.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        data.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(idx, indexCount);
                        indexCount += 4;
                    }

                    //Top face
                    BlockType blockTop = getBlockAt(i, std::min(255, j + 1), k);
                    if (blockTop == EMPTY || blockTop == WATER ||
                            blockTop == ICE || j == 255) {
                        // Top face positions
                        // Top face normal is +j
                        glm::vec4 norm = glm::vec4(0.f, 1.f, 0.f, 0.f);
                        glm::vec4 uv = getUVs(t, YPOS);
                        //UL
                        data.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        data.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv);
                        //LR
                        data.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        data.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                        data.push_back(norm);
                        data.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(idx, indexCount);
                        indexCount += 4;
                    }
                } else if (t == WATER || t == ICE) { // Transparent blocks
                    // Back face (face with LL vertex at worldPos)
                    BlockType blockBehind = getBlockAt(i, j, std::max(0, k - 1));
                    if (k == 0) {
                        if (m_neighbors.at(ZNEG) != nullptr) {
                            blockBehind = m_neighbors.at(ZNEG)->getBlockAt(i, j, 15);
                        }
                    }
                    if (blockBehind == EMPTY || (k == 0 && m_neighbors.at(ZNEG) == nullptr)) {
                        // Back face positions
                        // Back face normal is -k
                        glm::vec4 norm = glm::vec4(0.f, 0.f, -1.f, 0.f);
                        glm::vec4 uv = getUVs(t, ZNEG);
                        //UL
                        tData.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        tData.push_back(worldPos);
                        tData.push_back(glm::vec4(0.f, 0.f, -1.f, 0.f));
                        tData.push_back(uv);
                        //LR
                        tData.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                        tData.push_back(glm::vec4(0.f, 0.f, -1.f, 0.f));
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        tData.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                        tData.push_back(glm::vec4(0.f, 0.f, -1.f, 0.f));
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(tIdx, tIndexCount);
                        tIndexCount += 4;
                    }

                    // Front face
                    BlockType blockFront = getBlockAt(i, j, std::min(15, k + 1));
                    if (k == 15) {
                        if (m_neighbors.at(ZPOS) != nullptr) {
                            blockFront = m_neighbors.at(ZPOS)->getBlockAt(i, j, 0);
                        }
                    }
                    if (blockFront == EMPTY || (k == 15 && m_neighbors.at(ZPOS) == nullptr)) {
                        // Front face positions
                        // Front face normal is +k
                        glm::vec4 norm = glm::vec4(0.f, 0.f, 1.f, 0.f);
                        glm::vec4 uv = getUVs(t, ZPOS);
                        //UL
                        tData.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        tData.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv);
                        //LR
                        tData.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        tData.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(tIdx, tIndexCount);
                        tIndexCount += 4;
                    }

                    // Left face
                    BlockType blockLeft = getBlockAt(std::max(0, i - 1), j, k);
                    if (i == 0) {
                        if (m_neighbors.at(XNEG) != nullptr) {
                            blockLeft = m_neighbors.at(XNEG)->getBlockAt(15, j, k);
                        }
                    }
                    if (blockLeft == EMPTY || (i == 0 && m_neighbors.at(XNEG) == nullptr)) {
                        // Left face positions
                        // Left face normal is -i
                        glm::vec4 norm = glm::vec4(-1.f, 0.f, 0.f, 0.f);
                        glm::vec4 uv = getUVs(t, XNEG);
                        //UL
                        tData.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        tData.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv);
                        //LR
                        tData.push_back(worldPos);
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        tData.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(tIdx, tIndexCount);
                        tIndexCount += 4;
                    }

                    // Right face
                    BlockType blockRight = getBlockAt(std::min(15, i + 1), j, k);
                    if (i == 15) {
                        if (m_neighbors.at(XPOS) != nullptr) {
                            blockRight = m_neighbors.at(XPOS)->getBlockAt(0, j, k);
                        }
                    }
                    if (blockRight == EMPTY || (i == 15 && m_neighbors.at(XPOS) == nullptr)) {
                        // Right face positions
                        // Right face normal is +i
                        glm::vec4 norm = glm::vec4(1.f, 0.f, 0.f, 0.f);
                        glm::vec4 uv = getUVs(t, XPOS);
                        //UL
                        tData.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        tData.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv);
                        //LR
                        tData.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        tData.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(tIdx, tIndexCount);
                        tIndexCount += 4;
                    }

                    // Bottom face
                    BlockType blockBottom = getBlockAt(i, std::max(0, j - 1), k);
                    if (blockBottom == EMPTY || j == 0) {
                        // Bottom face positions
                        // Bottom face normal is -j
                        glm::vec4 norm = glm::vec4(0.f, -1.f, 0.f, 0.f);
                        glm::vec4 uv = getUVs(t, YNEG);
                        //UL
                        tData.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        tData.push_back(worldPos);
                        tData.push_back(norm);
                        tData.push_back(uv);
                        //LR
                        tData.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        tData.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(tIdx, tIndexCount);
                        tIndexCount += 4;
                    }

                    //Top face
                    BlockType blockTop = getBlockAt(i, std::min(255, j + 1), k);
                    if (blockTop == EMPTY || j == 255) {
                        // Top face positions
                        // Top face normal is +j
                        glm::vec4 norm = glm::vec4(0.f, 1.f, 0.f, 0.f);
                        glm::vec4 uv = getUVs(t, YPOS);
                        //UL
                        tData.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(0.f, 1.f / 16.f, 0.f, 0.f));
                        //LL
                        tData.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv);
                        //LR
                        tData.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 0.f, 0.f, 0.f));
                        //UR
                        tData.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                        tData.push_back(norm);
                        tData.push_back(uv + glm::vec4(1.f / 16.f, 1.f / 16.f,
                                                      0.f, 0.f));
                        // Add indices
                        pushIndexForFace(tIdx, tIndexCount);
                        tIndexCount += 4;
                    }
                }
            }
        }
    }
}


void Chunk::pushIndexForFace(std::vector<GLuint>&idx, int index)
{
    idx.push_back(index);
    idx.push_back(index + 1);
    idx.push_back(index + 2);
    idx.push_back(index);
    idx.push_back(index + 2);
    idx.push_back(index + 3);
}

glm::vec4 Chunk::getUVs(BlockType &type, Direction face)
{
    if (type == DIRT) {
        return glm::vec4(2.f, 15.f, 0.f, 0.f) / 16.f;
    } else if (type == STONE) {
        return glm::vec4(1.f, 15.f, 0.f, 0.f) / 16.f;
    } else if (type == GRASS) {
        if (face == YPOS) {
            return glm::vec4(8.f, 13.f, 0.f, 0.f) / 16.f;
        } else {
            return glm::vec4(3.f, 15.f, 0.f, 0.f) / 16.f;
        }
    } else if (type == LAVA) {
        return glm::vec4(13.f, 1.f, 0.f, 0.f) / 16.f;
    } else if (type == WATER) {
        return glm::vec4(13.f, 3.f, 0.f, 0.f) / 16.f;
    } else if (type == ICE) {
        return glm::vec4(3.f, 11.f, 0.f, 0.f) / 16.f;
    } else if (type == SNOW) {
        return glm::vec4(2.f, 11.f, 0.f, 0.f) / 16.f;
    }
}

void Chunk::bufferToDrawableVBOs()
{
    m_count = this->idx.size();

    // Generate index buffer
    generateIdx();
    // Bind index buffer
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Buffer index data
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof (GLuint), this->idx.data(), GL_STATIC_DRAW);
    // Generate data buffer
    generateAll();
    // Bind data buffer
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_buffAll);
    // Buffer data to GPU
    mp_context->glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec4), this->data.data(), GL_STATIC_DRAW);
}

void Chunk::bufferTransparentDrawableVBOs()
{
    m_count = this->tIdx.size();

    // Generate index buffer
    generateIdx();
    // Bind index buffer
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    // Buffer index data
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof (GLuint), this->tIdx.data(), GL_STATIC_DRAW);
    // Generate data buffer
    generateAll();
    // Bind data buffer
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_buffAll);
    // Buffer data to GPU
    mp_context->glBufferData(GL_ARRAY_BUFFER, tData.size() * sizeof(glm::vec4), this->tData.data(), GL_STATIC_DRAW);
}

bool Chunk::hasXPOSneighbor() { return m_neighbors.at(XPOS) != nullptr; }
bool Chunk::hasXNEGneighbor() { return m_neighbors.at(XNEG) != nullptr; }
bool Chunk::hasZPOSneighbor() { return m_neighbors.at(ZPOS) != nullptr; }
bool Chunk::hasZNEGneighbor() { return m_neighbors.at(ZNEG) != nullptr; }


