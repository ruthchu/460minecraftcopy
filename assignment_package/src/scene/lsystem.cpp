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
      ruleMap(QHash<QChar, Rule>()), terrain(terrain), inputPosition(position)
{}

void Lsystem::setRiverStart()
{
    srand((unsigned) time(0));
    //int result = rand() % 4;
    int result = 0;
    float riverY = 130.f;
    float offset = 2.f;
    //float result = Noise::random1(glm::vec2(inputPosition[0], inputPosition[1]));

    glm::vec2 LLcorner = this->terrain.getTerrainAt(inputPosition[0], inputPosition[1]);
    // Randomly select a corner of the terrain zone
    if (result == 0) {
        currentTurtle.pos = glm::vec3(LLcorner.x + offset, riverY, LLcorner.y + offset);
        currentTurtle.orient = glm::vec3(1, 0, 1);
    } else if (result == 1) {
        glm::vec2 ULcorner = LLcorner + glm::vec2(0.f, BLOCK_LENGTH_IN_TERRAIN);
        currentTurtle.pos = glm::vec3(ULcorner.x + offset, riverY, ULcorner.y - offset);
        currentTurtle.orient = glm::vec3(1, 0, -1);
    } else if (result == 2) {
        glm::vec2 LRcorner = LLcorner + glm::vec2(BLOCK_LENGTH_IN_TERRAIN, 0.f);
        currentTurtle.pos = glm::vec3(LRcorner.x - offset, riverY, LRcorner.y + offset);
        currentTurtle.orient = glm::vec3(-1, 0, 1);
    } else {
        glm::vec2 URcorner = LLcorner + glm::vec2(BLOCK_LENGTH_IN_TERRAIN, BLOCK_LENGTH_IN_TERRAIN);
        currentTurtle.pos = glm::vec3(URcorner.x - offset, riverY, URcorner.y - offset);
        currentTurtle.orient = glm::vec3(-1, 0, 1);
    }
    // set segement length
    currentTurtle.length = 10.0f;
    // set diameter of river
    currentTurtle.depth = 4.f;
}

void Lsystem::makeRivers()
{
    // randomly decide whether or not create river with 0.6 chance
//    float result = Noise::random1(glm::vec2(inputPosition[0], inputPosition[1]));
//    if (result < 0.33) {
//        return;
//    }
    return;

    setRiverStart();

    grammarMap[QChar('X')] = QString("F+[+++F-F+X-FFX]--FF++F+FX");
    grammarMap[QChar('F')] = QString("F-B");
    grammarMap[QChar('B')] = QString("XX");

    void (Lsystem::*fPtr)(void);
    fPtr = &Lsystem::fRule;
    ruleMap[QChar('F')] = fPtr;

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

    srand((unsigned) time(0));
    float prob = rand() / RAND_MAX;
    float iter = 0;
//    if (prob < 0.2) {
        iter = 1;
//    } else if (prob < 0.70) {
//        iter = 2;
//    } else {
//        iter = 3;
//    }

    QString q = strMaker(2, "FX");
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
                    ((first == QChar(']')) && (sec == QChar('+')) && (third == QChar('F'))) ||
                    ((first == QChar(']')) && (sec == QChar('-')) && (third == QChar('F')));
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
    float ran = (rand() / RAND_MAX) * 2 - 1;
    float angle = -20.f + (ran * 4);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 newOrient = rotation * glm::vec4(this->currentTurtle.orient, 1.f);
    this->currentTurtle.orient = glm::vec3(newOrient.x, newOrient.y, newOrient.z);
}

void Lsystem::rotateLeft()
{
    srand((unsigned) time(0));
    float ran = (rand() / RAND_MAX) * 2 - 1;
    float angle = 20.f + (ran * 4);
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
        this->currentTurtle.depth = std::max(this->currentTurtle.depth - 1, 2.f);
        currentTurtle.isNewBranch = false;
    }

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
                        this->terrain.setBlockAt(x, y, z, WATER);
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
//    srand((unsigned) time(0));
//    float ran = (rand() / RAND_MAX) * 2 - 1;
//    this->currentTurtle.length = currentTurtle.length + ran * currentTurtle.length;
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







