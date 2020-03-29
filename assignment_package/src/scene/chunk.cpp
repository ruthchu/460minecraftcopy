#include "chunk.h"
#include <openglcontext.h>
#include <glm_includes.h>


Chunk::Chunk(OpenGLContext* context, int X, int Z)
    : Drawable(context), X(X), Z(Z), m_blocks(),
      m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
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
    //GLuint m_bufVerData;
    //mp_context->glGenBuffers(1, &m_bufVerData);
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> col;
    std::vector<glm::vec4> norm;
    std::vector<GLuint> idx;

    int index = 0;
    // Iterate over all blocks in chunk
    for (int i = 0; i < 16; i++) { // x
        for (int j = 0; j < 256; j++) { // y
            for (int k = 0; k < 16; k++) { // z
                // Block at current location
                BlockType t = getBlockAt(i, j, k);
                if (t == EMPTY) {
                    continue;
                }

                // Number of color attributes
                int numColor = 0;
                // Block's local position
                glm::vec4 localPos = glm::vec4(i, j, k, 1.f);
                // Block's bottom left corner in world space
                glm::vec4 worldPos = glm::vec4(this->X, 0.f, this->Z, 0.f) + localPos;

                // Back face (face with LL vertex at worldPos)
                BlockType blockBehind = getBlockAt(i, j, std::max(0, k - 1));
                if (blockBehind == EMPTY && k != 0) {
                    // Back face positions
                    //UL
                    pos.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                    //LL
                    pos.push_back(worldPos);
                    //UR
                    pos.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                    //LR
                    pos.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                    // Back face normal is -k
                    pushNormal(norm, glm::vec4(0.f, 0.f, -1.f, 0.f), 4);
                    numColor += 4;
                    // Add indices
                    pushIndexForFace(idx, index);
                    index += 4;
                }

                // Front face
                BlockType blockFront = getBlockAt(i, j, std::min(16, k + 1));
                if (blockFront == EMPTY && k != 16) {
                    // Front face positions
                    //UL
                    pos.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                    //LL
                    pos.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                    //UR
                    pos.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                    //LR
                    pos.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                    // Front face normal is +k
                    pushNormal(norm, glm::vec4(0.f, 0.f, 1.f, 0.f), 4);
                    numColor += 4;
                    // Add indices
                    pushIndexForFace(idx, index);
                    index += 4;
                }

                // Left face
                BlockType blockLeft = getBlockAt(std::max(0, i - 1), j, k);
                if (blockLeft == EMPTY && i != 0) {
                    // Left face positions
                    //UL
                    pos.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                    //LL
                    pos.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                    //UR
                    pos.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                    //LR
                    pos.push_back(worldPos);
                    // Left face normal is -i
                    pushNormal(norm, glm::vec4(-1.f, 0.f, 0.f, 0.f), 4);
                    numColor += 4;
                    // Add indices
                    pushIndexForFace(idx, index);
                    index += 4;
                }

                // Right face
                BlockType blockRight = getBlockAt(std::min(16, i + 1), j, k);
                if (blockRight == EMPTY && i != 16) {
                    // Right face positions
                    //UL
                    pos.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                    //LL
                    pos.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                    //UR
                    pos.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                    //LR
                    pos.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                    // Right face normal is +i
                    pushNormal(norm, glm::vec4(1.f, 0.f, 0.f, 0.f), 4);
                    numColor += 4;
                    // Add indices
                    pushIndexForFace(idx, index);
                    index += 4;
                }

                // Bottom face
                BlockType blockBottom = getBlockAt(i, std::max(0, j - 1), k);
                if (blockBottom == EMPTY || j == 0) {
                    // Bottom face positions
                    //UL
                    pos.push_back(worldPos + glm::vec4(0.f, 0.f, 1.f, 0.f));
                    //LL
                    pos.push_back(worldPos);
                    //UR
                    pos.push_back(worldPos + glm::vec4(1.f, 0.f, 1.f, 0.f));
                    //LR
                    pos.push_back(worldPos + glm::vec4(1.f, 0.f, 0.f, 0.f));
                    // Bottom face normal is -j
                    pushNormal(norm, glm::vec4(0.f, -1.f, 0.f, 0.f), 4);
                    numColor += 4;
                    // Add indices
                    pushIndexForFace(idx, index);
                    index += 4;
                }

                // Top face
                BlockType blockTop = getBlockAt(i, std::min(j + 1, 255), k);
                if (blockTop == EMPTY || j == 255) {
                    // Top face positions
                    //UL
                    pos.push_back(worldPos + glm::vec4(0.f, 1.f, 1.f, 0.f));
                    //LL
                    pos.push_back(worldPos + glm::vec4(0.f, 1.f, 0.f, 0.f));
                    //UR
                    pos.push_back(worldPos + glm::vec4(1.f, 1.f, 1.f, 0.f));
                    //LR
                    pos.push_back(worldPos + glm::vec4(1.f, 1.f, 0.f, 0.f));
                    // Top face normal is +j
                    pushNormal(norm, glm::vec4(0.f, 1.f, 0.f, 0.f), 4);
                    numColor += 4;
                    // Add indices
                    pushIndexForFace(idx, index);
                    index += 4;
                }

                // Push back colors
                pushColor(col, t, numColor);
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

void Chunk::pushNormal(std::vector<glm::vec4>&norm, glm::vec4 dir, int amount)
{
    for (int i = 0; i < amount; i++) {
        norm.push_back(dir);
    }
}

void Chunk::pushColor(std::vector<glm::vec4>&col, BlockType type, int amount)
{
    for (int i = 0; i < amount; i++) {
        switch(type) {
        case GRASS:
            col.push_back(glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f);
            break;
        case DIRT:
            col.push_back(glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f);
            break;
        case STONE:
            col.push_back(glm::vec4(0.5f));
            break;
        default:
            // Other block types are not yet handled, so we default to black
            col.push_back(glm::vec4(0.f));
            break;
        }
    }
}

std::vector<glm::vec4> Chunk::combineData(std::vector<std::vector<glm::vec4>> data)
{
}
