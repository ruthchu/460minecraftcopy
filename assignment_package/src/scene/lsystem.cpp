#include "lsystem.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <algorithm>
#include "noise.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Lsystem::Lsystem(Terrain &terrain, glm::ivec2 position)
    : currentTurtle(Turtle(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0.f, 0.f)),
      tStack(std::stack<Turtle>()), grammarMap(QHash<QChar, QString>()),
      ruleMap(QHash<QChar, Rule>()), terrain(terrain), inputPosition(position),
      riverType(WATER)
{}

void Lsystem::setRiverStart()
{
    srand((unsigned) time(0));
    float riverY = 129.f;
    float offset = 2.f;
    float result = Noise::random1(glm::vec2(inputPosition[0], inputPosition[1]));

    glm::vec2 LLcorner = this->terrain.getTerrainAt(inputPosition[0], inputPosition[1]);
    // Randomly select a corner of the terrain zone
    if (result > 0.75f) {
        currentTurtle.pos = glm::vec3(LLcorner.x + offset, riverY, LLcorner.y + offset);
        currentTurtle.orient = glm::vec3(1, 0, 1);
    } else if (result > 0.5f) {
        glm::vec2 ULcorner = LLcorner + glm::vec2(0.f, BLOCK_LENGTH_IN_TERRAIN);
        currentTurtle.pos = glm::vec3(ULcorner.x + offset, riverY, ULcorner.y - offset);
        currentTurtle.orient = glm::vec3(1, 0, -1);
    } else if (result > 0.25f) {
        glm::vec2 LRcorner = LLcorner + glm::vec2(BLOCK_LENGTH_IN_TERRAIN, 0.f);
        currentTurtle.pos = glm::vec3(LRcorner.x - offset, riverY, LRcorner.y + offset);
        currentTurtle.orient = glm::vec3(-1, 0, 1);
    } else {
        glm::vec2 URcorner = LLcorner + glm::vec2(BLOCK_LENGTH_IN_TERRAIN, BLOCK_LENGTH_IN_TERRAIN);
        currentTurtle.pos = glm::vec3(URcorner.x - offset, riverY, URcorner.y - offset);
        currentTurtle.orient = glm::vec3(-1, 0, 1);
    }

    // set segement length
    // set diameter of river
    int m = ((int) result * 100) % 3;
    if (m == 0) {
        currentTurtle.length = 3.f;
    } else if (m == 1) {
        currentTurtle.length = 4.f;
    } else {
        currentTurtle.length = 5.f;
    }

    int n = Noise::random1(glm::vec2(inputPosition[0] + 9, inputPosition[1] + 9));
    std::cout << "n " << n << std::endl;
    if (n == 0) {
        currentTurtle.depth = 6.f;
    } else if (n == 1) {
        currentTurtle.depth = 4.f;
    } else {
        currentTurtle.depth = 3.f;
    }
}

void Lsystem::makeLava() {
    riverType = LAVA;
}

void Lsystem::makeRivers()
{
    int noise = Noise::random1(glm::vec2(inputPosition[0], inputPosition[1]));
    std::cout << noise << std::endl;
    if (noise > 0.6) {
        return;
    }
    setRiverStart();

    grammarMap[QChar('A')] = QString("AGK");
    grammarMap[QChar('M')] = QString("B+A[AG]-AG");
    grammarMap[QChar('B')] = QString("[K+[AGK]G+G+K]-K");
    grammarMap[QChar('G')] = QString("A+A");
    grammarMap[QChar('K')] = QString("A-A");

    void (Lsystem::*fPtr)(void);
    fPtr = &Lsystem::fRule;
    ruleMap[QChar('A')] = fPtr;

    void (Lsystem::*popPtr)(void);
    popPtr = &Lsystem::popState;
    ruleMap[QChar(']')] = popPtr;

    void (Lsystem::*pushPtr)(void);
    pushPtr = &Lsystem::saveState;
    ruleMap[QChar('[')] = pushPtr;

    void (Lsystem::*rotRight)(void);
    rotRight = &Lsystem::rotateRight;
    ruleMap[QChar('+')] = rotRight;

    void (Lsystem::*rotLeft)(void);
    rotLeft = &Lsystem::rotateLeft;
    ruleMap[QChar('+')] = rotLeft;

    noise = Noise::random1(glm::vec2(inputPosition[0] + 4, inputPosition[1] + 2));
    float iter = 2;
    if (noise > 0.5) {
        iter = 3;
    }

    srand((unsigned) time(0));
    if (rand() % 4 == 0) {
        makeLava();
    }

    QString q = strMaker(iter, "AB+G-K+M");
    //std::cout << q.toUtf8().constData() << std::endl;
    lsystemParser(q);
}

void Lsystem::lsystemParser(QString str)
{
    // Used to keep track of branching
    QChar first = QChar('H');
    QChar sec = QChar('H');
    QChar third = QChar('H');
    for (int i = 0; i < str.size(); i++) {
        first = sec;
        sec = third;
        third = QChar(str[i]);
        if (ruleMap.contains(QChar(str[i]))) {
            currentTurtle.isNewBranch = // pop, rot, F creates new branch
                    ((first == QChar(']')) && (sec == QChar('+')) && (third == QChar('A'))) ||
                    ((first == QChar(']')) && (sec == QChar('-')) && (third == QChar('A')));
            void (Lsystem::*drawingFunction) (void) = this->ruleMap[QChar(str[i])];
            (this->*drawingFunction)();
        }
    }
}

QString Lsystem::strMaker(int iterations, const QString axiom)
{
    if (iterations == 0) {
        //std::cout << axiom.toUtf8().constData() << std::endl;
        return QString(axiom);
    } else {
        iterations--;
        QString newAxiom;
        for (int i = 0; i < axiom.size(); i++) {
            // TODO -- use a noise function for add diff probabilities
            newAxiom.append(grammarMap[QChar(axiom[i])]);
        }
        //std::cout << newAxiom.toUtf8().constData() << std::endl;
        return strMaker(iterations, newAxiom);
    }
}

void Lsystem::saveState()
{
    tStack.push(currentTurtle);
}

void Lsystem::popState()
{
    if (!tStack.empty()) {
        Turtle &temp = tStack.top();
        currentTurtle = Turtle(temp);
        tStack.pop();
    }
}

void Lsystem::rotateRight()
{

    srand((unsigned) time(0));
    float x = rand() % 5;
    float angle = -20.f - x;
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 newOrient = rotation * glm::vec4(this->currentTurtle.orient, 1.f);
    this->currentTurtle.orient = glm::vec3(newOrient.x, newOrient.y, newOrient.z);
}

void Lsystem::rotateLeft()
{
    srand((unsigned) time(0));
    float x = rand() % 5;
    float angle = 20.f + x;
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 newOrient = rotation * glm::vec4(this->currentTurtle.orient, 1.f);
    this->currentTurtle.orient = glm::vec3(newOrient.x, newOrient.y, newOrient.z);
}

void Lsystem::fRule()
{
//    std::cout << "Current Turtle Pos: "<< this->currentTurtle.pos.x << " " <<
//              this->currentTurtle.pos.y <<  " " << this->currentTurtle.pos.z << std::endl;
//    std::cout << "Current Turtle Orientation: "<< this->currentTurtle.orient.x << " " <<
//                 this->currentTurtle.orient.y <<  " " << this->currentTurtle.orient.z << std::endl;
//    std::cout << "Current Turtle Depth: "<< this->currentTurtle.depth << std::endl;
//    std::cout << "Current Turtle Length: "<< this->currentTurtle.length << std::endl;
//    std::cout << " " << std::endl;

    if (currentTurtle.isNewBranch) {
        this->currentTurtle.depth = std::max(this->currentTurtle.depth - 1, 0.f);
        currentTurtle.isNewBranch = false;
    }

    if (this->currentTurtle.depth == 0) {return;}

    float capsuelRad = this->currentTurtle.depth;
    float capsuleCenterY = this->currentTurtle.pos.y;
    float waterLevel = 128.f;

    glm::vec3 a = this->currentTurtle.pos;
    glm::vec3 b = this->currentTurtle.pos + this->currentTurtle.length * glm::normalize(this->currentTurtle.orient);

    // If drawing the segment will leave the terrain zone, do not draw it
    if (!isInZone(b)) return;

    float xmin = std::min(a.x - capsuelRad, b.x - capsuelRad);
    float xmax = std::max(a.x + capsuelRad, b.x + capsuelRad);
    float zmin = std::min(a.z - capsuelRad, b.z - capsuelRad);
    float zmax = std::min(a.z + capsuelRad, b.z + capsuelRad);

    for (int x = xmin; x <= xmax; x++) {
        for (int z = zmin; z <= zmax; z++) {
            for (int y = capsuleCenterY - capsuelRad; y <= capsuleCenterY; y++) {
                // If no chunk at location, do not check
                if (!this->terrain.hasChunkAt(x, z)) continue;
                // Check if block is inside the capsule
                if (sdCapsule(glm::vec3(x, y, z), a, b, capsuelRad) <= 0.f) {
                    // If y <= waterlevel, put water block
                    if (y <= waterLevel) {
                        this->terrain.setBlockAt(x, y, z, riverType);
                    } else if (y < capsuleCenterY) { // If y > waterlevel but < capsuleCenterY put empty
                        this->terrain.setBlockAt(x, y, z, EMPTY);
                    } else if (y == capsuleCenterY) { // If y == capsuleCenterY, carve out everything on top
                        for (int y = capsuleCenterY; y < 256; y++) {
                            this->terrain.setBlockAt(x, y, z, EMPTY);
                        }
                    }
                }
            }
        }
    }

    // update current turtle
    srand((unsigned) time(0));
    float x = rand() % 4;
    if (x == 0) {
        currentTurtle.length = 10.f;
    } else if (x == 1) {
        currentTurtle.length = 7.f;
    } else if (x == 2) {
        currentTurtle.length = 5.f;
    } else {
        currentTurtle.length = 3.f;
    }
    this->currentTurtle = Turtle(b, this->currentTurtle.orient, this->currentTurtle.length, this->currentTurtle.depth);
}

// a and b are endpoints. p is the point u are checking, returns <= 0 if point is inside capsul
float Lsystem::sdCapsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r)
{
  glm::vec3 pa = p - a, ba = b - a;
  float dot = glm::dot(pa,ba)/glm::dot(ba,ba);
  float h = glm::clamp(dot, 0.0f, 1.0f);
  return glm::length( pa - ba*h ) - r;
}

bool Lsystem::isInZone(glm::vec3 p) {
    // X and Z coordinates of the LL corner of the terrain
    glm::vec2 LLcorner = this->terrain.getTerrainAt(inputPosition[0], inputPosition[1]);
    return (p.x > LLcorner[0]) && (p.x < LLcorner[0] + BLOCK_LENGTH_IN_TERRAIN) &&
            (p.z > LLcorner[1]) && (p.z < LLcorner[1] + BLOCK_LENGTH_IN_TERRAIN);
}







