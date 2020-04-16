#include "lsystem.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

Lsystem::Lsystem(Terrain &terrain)
    : currentTurtle(Turtle(glm::vec4(0, 0, 0, 0), 0.f, 20.0f, 12.f)),
      tStack(std::stack<Turtle>()), grammarMap(QHash<QChar, QString>()),
      ruleMap(QHash<QChar, Rule>()), terrain(terrain)
{}

void Lsystem::makeRivers()
{
    grammarMap[QChar('X')] = QString("F+[+++F-F+X-FFX]--FF+F+FX");
    grammarMap[QChar('F')] = QString("F-B");
    grammarMap[QChar('B')] = QString("XX");
    void (Lsystem::*fPtr)(void);
    fPtr = &Lsystem::fRule;
    ruleMap[QChar('F')] = fPtr;
    QString q = strMaker(1, "FX");
    lsystemParser(q);
}

void Lsystem::lsystemParser(QString str)
{
    for (int i = 0; i < str.size(); i++) {
        if (ruleMap.contains(QChar(str[i]))) {
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
    // TODO -- use a noise function for random rotation
    float angle = 10.f;
    float sign = (currentTurtle.orient + angle) / (currentTurtle.orient + angle);
    currentTurtle.orient = sign * fmod((currentTurtle.orient + angle), 360.f);
}

void Lsystem::rotateLeft()
{
    // TODO -- use a noise function for random rotation
    float angle = 10.f;
    float sign = (currentTurtle.orient + angle) / (currentTurtle.orient + angle);
    currentTurtle.orient = sign * fmod((currentTurtle.orient - angle), 360.f);
}

void Lsystem::fRule()
{
    //std::cout << "frule" << std::endl;
    float capsuelRad = this->currentTurtle.depth;
    float capsuleCenterY = 135.f;
    float waterLevel = 128.f;
    glm::vec4 tPos = this->currentTurtle.pos;
    glm::vec3 a = glm::vec3(tPos.x, capsuleCenterY, tPos.z);
    glm::vec4 newPos = currentTurtle.length *
            glm::rotate(glm::mat4(1.0f), glm::radians(currentTurtle.orient), glm::vec3(0.0f, 0.0f, 1.0f))
            * glm::normalize(glm::vec4(glm::vec4(tPos.x, capsuleCenterY, tPos.z, tPos.w)));
    glm::vec3 b = glm::vec3(newPos.x, newPos.y, newPos.z);

//    glm::vec3 b = glm::vec3(tPos.x, capsuleCenterY, tPos.z + this->currentTurtle.length);

//    float capsuelRad = 12.f;
//    float capsuleCenterY = 135.f;
//    float waterLevel = 128.f;
//    glm::vec3 a = glm::vec3(0, capsuleCenterY, 0);
//    glm::vec3 b = glm::vec3(40, capsuleCenterY, 40);

    for (int x = std::min(a.x - capsuelRad, b.x - capsuelRad); x < std::max(a.x + capsuelRad, b.x + capsuelRad); x++) {
        for (int z = std::min(a.z - capsuelRad, b.z - capsuelRad); z < std::max(a.z + capsuelRad, b.z + capsuelRad); z++) {
            for (int y = capsuleCenterY - capsuelRad; y <= capsuleCenterY + capsuelRad; y++) {
                if (!this->terrain.hasChunkAt(x, z)) continue;
                glm::vec3 p = glm::vec3(x, y, z);
                // Check if a block is inside the capsule
                if (sdCapsule(p, a, b, capsuelRad) <= 0.f) {
                    // Carve terrain
                    if (!this->terrain.hasChunkAt(p.x, p.z)) return;
                    if (p.y <= waterLevel) {
                        this->terrain.setBlockAt(p.x, p.y, p.z, WATER);
                        //std::cout << "adding water" << std::endl;
                    }
                    for (int y = waterLevel + 1; y < 256; y++) {
                        this->terrain.setBlockAt(p.x, y, p.z, EMPTY);
                    }
                }
            }
        }
    }

    // update current turtle
    // TODO -- randomize length, shrink deph
    currentTurtle = Turtle(newPos, this->currentTurtle.orient, this->currentTurtle.length, this->currentTurtle.depth);
}

// a and b are endpoints. p is the point u are checking, returns <= 0 if point is inside capsul
float Lsystem::sdCapsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r)
{
  glm::vec3 pa = p - a, ba = b - a;
  float dot = glm::dot(pa,ba)/glm::dot(ba,ba);
  float h = glm::clamp(dot, 0.0f, 1.0f);
  return glm::length( pa - ba*h ) - r;
}








