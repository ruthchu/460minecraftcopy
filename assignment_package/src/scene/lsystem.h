#ifndef LSYSTEM_H
#define LSYSTEM_H

#include <stack>
#include <iostream>
#include <map>
#include "turtle.h"
#include "terrain.h"
#include <QHash>

class Lsystem;
class Terrain;

typedef void (Lsystem::*Rule)(void);

class Lsystem
{
private:
    Turtle currentTurtle;
    std::stack<Turtle> tStack;
    QHash<QChar, QString> grammarMap;
    QHash<QChar, Rule> ruleMap;
    void saveState();
    void popState();
    void rotateRight();
    void rotateLeft();
    void fRule();
    float sdCapsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r);
    Terrain &terrain;
    // recursive grammar subsitution method
    QString strMaker(int iterations, const QString axiom);
    // reads a string and converts to grammar
    void lsystemParser(QString str);
    glm::ivec2 inputPosition;
    void setRiverStart();
    bool isInZone(glm::vec3 p);
public:
    Lsystem(Terrain &terrain, glm::ivec2 inputPosition);
    void makeRivers();
};

#endif // LSYSTEM_H
