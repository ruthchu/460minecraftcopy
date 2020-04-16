#include "turtle.h"

Turtle::Turtle(glm::vec3 pos, glm::vec3 orient, float length, float depth)
    : pos(pos), orient(orient), length(length), depth(depth)
{}

Turtle::Turtle(const Turtle& ref)
    : pos(ref.pos), orient(ref.orient), length(ref.length), depth(ref.depth)
{}
