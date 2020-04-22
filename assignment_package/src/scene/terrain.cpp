#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>
#include <glm/glm.hpp>

const static bool DEBUGMODE = true;

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context), test(false)
{}

Terrain::~Terrain() {
    //m_geomCube.destroy();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::createChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x <= maxX; x += BLOCK_LENGTH_IN_CHUNK) {
        for(int z = minZ; z <= maxZ; z += BLOCK_LENGTH_IN_CHUNK) {
            if (hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0, 0, 0)));
                chunk->bufferToDrawableVBOs();
                shaderProgram->draw(*chunk);
            }
        }
    }
    for(int x = minX; x <= maxX; x += BLOCK_LENGTH_IN_CHUNK) {
        for(int z = minZ; z <= maxZ; z += BLOCK_LENGTH_IN_CHUNK) {
            if (hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(0, 0, 0)));
                chunk->bufferTransparentDrawableVBOs();
                shaderProgram->draw(*chunk);
            }
        }
    }
}

void Terrain::CreateTestScene()
{
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            createChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));
}

void Terrain::createMoreTerrainAt(int xPos, int zPos)
{
    createChunkAt(xPos, zPos);

    // Fill chunk with procedural height and blocktype data
    for(int x = xPos; x < xPos + BLOCK_LENGTH_IN_CHUNK; ++x) {
        for(int z = zPos; z < zPos + BLOCK_LENGTH_IN_CHUNK; ++z) {

            int grass = heightGrassland(x, z);
            int mountain = heightMountain(x, z);
            float perlin = (Noise::perlinNoise(glm::vec2(float(x) / 64, float(z) / 64)) + 1) / 2.f;
            perlin = glm::smoothstep(0.25f, 0.75f, perlin);
            BlockType t;
            if (perlin > 0.5) {
                t = STONE; //stone
            } else {
                t = GRASS;// GRASS
            }
            int y = glm::mix(grass, mountain, perlin);
            setBlockAt(x, y, z, t);
            if (t == GRASS) {
                t = DIRT;
            }
            fillColumn(x, y - 1, z, t);
        }
    }
}

int Terrain::heightGrassland(int x, int z) {
    int baseHeight = 128;
    int heightRange = baseHeight / 8;
    float xNew = float(x) / 64.0f;
    float zNew = float(z) / 64.0f;
    float filterIdx = 0.50f;
    glm::vec2 uv = glm::vec2(xNew, zNew);
    float y = std::pow(Noise::worleyNoise(uv), filterIdx);
    y *= heightRange;
    y += baseHeight;
    return y;
}

int Terrain::heightMountain(int x, int z) {
    int baseHeight = 140;
    int heightRange = 255 - baseHeight;
    float xNew = float(x) / 64.0f;
    float zNew = float(z) / 64.0f;
    float freq = 2.5f;
    glm::vec2 uv = glm::vec2(xNew, zNew);
    glm::vec2 offset = glm::vec2(Noise::perlinNoise(uv),
                                 Noise::perlinNoise(uv + glm::vec2(5.2 + 1.3)));
    float y = (Noise::perlinNoise((uv + offset) * freq) * 0.5f) + 0.5f;
    float filterIdx = 1.0f;
    y = std::pow(y, filterIdx);
    y = (1.f - abs(y));
    y *= heightRange;
    y += baseHeight;
    return y;
}

void Terrain::fillColumn(int x, int y, int z, BlockType t) {
    int worldBaseHeight = 0;
    if (DEBUGMODE) {
        worldBaseHeight = y - 10;
    }
    for (int i = y; i >= worldBaseHeight; i--) {
        if (y <= 128) {
            t = STONE; //stone
        }
        setBlockAt(x, i, z, t);
    }
}

void Terrain::expandTerrainBasedOnPlayer(glm::vec3 pos)
{
    glm::ivec2 centerTerrain = this->getTerrainAt(pos.x, pos.z);
    int leftBound = centerTerrain[0] - BLOCK_LENGTH_IN_TERRAIN * TERRAIN_RADIUS;
    int rightBound = centerTerrain[0] + BLOCK_LENGTH_IN_TERRAIN * TERRAIN_RADIUS;
    int botBound = centerTerrain[1] - BLOCK_LENGTH_IN_TERRAIN * TERRAIN_RADIUS;
    int topBound = centerTerrain[1] + BLOCK_LENGTH_IN_TERRAIN * TERRAIN_RADIUS;

    for (int x = leftBound; x <= rightBound; x+= BLOCK_LENGTH_IN_TERRAIN) {
        for (int z = botBound; z <= topBound; z += BLOCK_LENGTH_IN_TERRAIN) {
            this->generateTerrainZone(x, z);
        }
    }


    // generate VBOs for each chunk with data
    chunksWithData.mu.lock();
    for (Chunk* c : chunksWithData.getVectorData()) {
        std::thread t(fillVBO, std::ref(*c), std::ref(this->chunksWithVBO));
        t.detach();
    }
    chunksWithData.clearChunkData();
    chunksWithData.mu.unlock();

    chunksWithVBO.mu.lock();
    chunksWithVBO.clearChunkData();
    chunksWithVBO.mu.unlock();

}

void Terrain::makeRivers(glm::ivec2 zonePosition)
{
    Lsystem lsystem = Lsystem(*this, zonePosition);
    lsystem.makeRivers();
}

glm::ivec2 Terrain::getTerrainAt(int x, int z) {
    int xFloor = glm::floor(x / 64.f) * BLOCK_LENGTH_IN_TERRAIN;
    int zFloor = glm::floor(z / 64.f) * BLOCK_LENGTH_IN_TERRAIN;
    return glm::vec2(xFloor, zFloor);
}

void Terrain::generateTerrainZone(int x, int z) {
    int64_t coord = toKey(x, z);
    if (this->m_generatedTerrain.find(coord) == this->m_generatedTerrain.end()) {
        // generate chunk data in terrain zone
        for (int i = 0; i <= BLOCK_LENGTH_IN_TERRAIN - BLOCK_LENGTH_IN_CHUNK; i += BLOCK_LENGTH_IN_CHUNK) {
            for (int j = 0; j <= BLOCK_LENGTH_IN_TERRAIN - BLOCK_LENGTH_IN_CHUNK; j += BLOCK_LENGTH_IN_CHUNK) {
                Chunk* cPtr = createChunkAt(x + i, z + j);
                std::thread t(fillBlockData, cPtr->X, cPtr->Z, cPtr, &this->chunksWithData);
                t.detach();
            }
        }
        makeRivers(glm::ivec2(x, z));
        this->m_generatedTerrain.insert(coord);
    }
}

void Terrain::fillBlockData(int xPos, int zPos, Chunk* chunk, BlockData *chunksWithData) {
    // Fill chunk with procedural height and blocktype data
    for(int x = xPos; x < xPos + BLOCK_LENGTH_IN_CHUNK; ++x) {
        for(int z = zPos; z < zPos + BLOCK_LENGTH_IN_CHUNK; ++z) {
            int grass = heightGrassland(x, z);
            int mountain = heightMountain(x, z);
            float perlin = (Noise::perlinNoise(glm::vec2(float(x) / 64, float(z) / 64)) + 1) / 2.f;
            perlin = glm::smoothstep(0.25f, 0.75f, perlin);
            BlockType t;
            if (perlin > 0.5) {
                t = STONE; //stone
            } else {
                t = GRASS; //GRASS
            }
            int y = glm::mix(grass, mountain, perlin);
            setBlockAtStatic(x, y, z, t, chunk);

            if (t == GRASS) {
                t = DIRT;
            }
            fillColumnStatic(x, y - 1, z, t, chunk);
        }
    }
    chunksWithData->addChunk(chunk);
}

void Terrain::setBlockAtStatic(int x, int y, int z, BlockType t, Chunk* c)
{
    glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
    c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                  static_cast<unsigned int>(y),
                  static_cast<unsigned int>(z - chunkOrigin.y),
                  t);
}

void Terrain::fillColumnStatic(int x, int y, int z, BlockType t, Chunk* c) {
    int worldBaseHeight = 0;
//    if (DEBUGMODE) {
//        worldBaseHeight = y - 4;
//    }
    for (int i = y; i >= worldBaseHeight; i--) {
        if (y <= 128) {
            t = STONE; //stone
        }
        setBlockAtStatic(x, i, z, t, c);
    }

}

void Terrain::fillVBO(Chunk &c, VBOCollection &chunksWithVBO) {
    c.create();
    chunksWithVBO.addChunk(&c);
}
