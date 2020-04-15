#ifndef TURTLE_H
#define TURTLE_H

#include <glm/glm.hpp>

class Turtle
{
public:
    glm::vec4 pos;
    float orient;
    float length;
    // radius of the capsule
    float depth;
    Turtle(glm::vec4 pos, float orient, float length, float depth);
    Turtle(const Turtle& ref);
};

#endif // TURTLE_H
