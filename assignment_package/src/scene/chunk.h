#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "drawable.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "texture.h"


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, SNOW, LAVA, WATER, ICE, SPIRE, SPIRE_TOP
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable
{
private:
    // Solid block data
    std::vector<GLuint> idx;
    std::vector<glm::vec4> data;

    // Transparent block data
    std::vector<GLuint> tIdx;
    std::vector<glm::vec4> tData;

    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    glm::vec4 getUVs(BlockType &type, Direction face);
    void pushIndexForFace(std::vector<GLuint>&idx, int index);
public:
    // Set up buffer for solid blocks
    void bufferToDrawableVBOs();
    // Set up buffer for transparent blocks
    void bufferTransparentDrawableVBOs();
    // Clear buffers
    void clearIdxBuffers();
    // Chunk's lower-left corner X and Z coordinates according to world
    int X;
    int Z;

    Chunk(OpenGLContext* context, int X, int Z);
    Chunk(OpenGLContext* context, int X, int Z, bool test);
    virtual ~Chunk(){};
    void create() override;

    BlockType getBlockAt(unsigned int X, unsigned int y, unsigned int Z) const;
    BlockType getBlockAt(int X, int y, int Z) const;
    void setBlockAt(unsigned int X, unsigned int y, unsigned int Z, BlockType t);
    void linkNeighbor(uPtr<Chunk> &neighbor, Direction dir);

    bool hasXPOSneighbor();
    bool hasXNEGneighbor();
    bool hasZPOSneighbor();
    bool hasZNEGneighbor();
};
