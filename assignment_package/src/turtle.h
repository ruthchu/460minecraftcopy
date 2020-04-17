#ifndef TURTLE_H
#define TURTLE_H

#include <glm/glm.hpp>

class Turtle
{
public:
    glm::vec3 pos;
    glm::vec3 orient;
    float length;
    // radius of the capsule
    float depth;
    bool isNewBranch;
    Turtle(glm::vec3 pos, glm::vec3 orient, float length, float depth);
    Turtle(const Turtle& ref);
};

#endif // TURTLE_H
