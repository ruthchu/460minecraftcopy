#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), m_geomCube(context), mp_context(context)
{}

Terrain::~Terrain() {
    m_geomCube.destroy();
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
    uPtr<Chunk> chunk = mkU<Chunk>();
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            for(int i = 0; i < 16; ++i) {
                for(int j = 0; j < 256; ++j) {
                    for(int k = 0; k < 16; ++k) {
                        BlockType t = chunk->getBlockAt(i, j, k);
                        switch(t) {
                        case GRASS:
                            shaderProgram->setGeometryColor(glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f);
                            break;
                        case DIRT:
                            shaderProgram->setGeometryColor(glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f);
                            break;
                        case STONE:
                            shaderProgram->setGeometryColor(glm::vec4(0.5f));
                            break;
                        default:
                            // Other block types are not yet handled, so we default to black
                            shaderProgram->setGeometryColor(glm::vec4(0.f));
                            break;
                        }
                        if(t != EMPTY) {
                            shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(i + x, j, k + z)));
                            shaderProgram->draw(m_geomCube);
                        }
                    }
                }
            }
        }
    }
}

void Terrain::CreateTestScene()
{
    // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!
    m_geomCube.create();

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

//    // Create the basic terrain floor
//    for(int x = 0; x < 64; ++x) {
//        for(int z = 0; z < 64; ++z) {
//            if((x + z) % 2 == 0) {
//                setBlockAt(x, 128, z, STONE);
//            }
//            else {
//                setBlockAt(x, 128, z, DIRT);
//            }
//        }
//    }

    //grassland test
//    for(int x = 0; x < 64; ++x) {
//        for(int z = 0; z < 64; ++z) {
//            int y = heightGrassland(x, z);
//            setBlockAt(x, y, z, GRASS);
//            fillColumn(x, y - 1, z, DIRT);
//        }
//    }

    //mountain test
    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            int y = heightMountain(x, z);
            setBlockAt(x, y, z, STONE);
            for (int i = 1; i < 10; i++) {
                setBlockAt(x, y - i, z, STONE);
            }
//            fillColumn(x, y, z, STONE);
        }
    }

//    // Add "walls" for collision testing
//    for(int x = 0; x < 64; ++x) {
//        setBlockAt(x, 129, 0, GRASS);
//        setBlockAt(x, 130, 0, GRASS);
//        setBlockAt(x, 129, 63, GRASS);
//        setBlockAt(0, 130, x, GRASS);
//    }
//    // Add a central column
//    for(int y = 129; y < 140; ++y) {
//        setBlockAt(32, y, 32, GRASS);
//    }
}

int Terrain::heightGrassland(int x, int z) {
    int baseHeight = 128;
    int heightRange = 128 / 8;
    float xNew = float(x) / 64.0f;
    float zNew = float(z) / 64.0f;
    float filterIdx = 0.90f;
    glm::vec2 uv = glm::vec2(xNew, zNew);
    float y = std::pow(Noise::worleyNoise(uv), filterIdx);
    y *= heightRange;
    y += baseHeight;
    return y;
}

int Terrain::heightMountain(int x, int z) {
    int baseHeight = 150;
    int heightRange = 255 - baseHeight;
    float xNew = float(x) / 64.0f;
    float zNew = float(z) / 64.0f;
    float freq = 2.0f;
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

int Terrain::blendMountainGrass(int grassHeight, int mountainHeight) {
    float noiseScalar = Noise::perlinNoise(glm::vec2(grassHeight, mountainHeight));
    // remap to [0,1]
    noiseScalar = (noiseScalar + 1) / 2.f;
    noiseScalar = glm::smoothstep(0.25f, 0.75f, noiseScalar);
    return (1 - noiseScalar) * grassHeight + noiseScalar * mountainHeight;
}

void Terrain::fillColumn(int x, int y, int z, BlockType t) {
    for (int i = y; i > 150; i--) {
        BlockType bt = t;
//        if (y >= 255 - 25) {
//            bt = SNOW;
//        }
        if (y <= 128) {
            bt = STONE;
        }
       setBlockAt(x, i, z, bt);
    }
}
