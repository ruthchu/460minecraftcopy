#pragma once

#include "drawable.h"
#include <glm_includes.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Cube : public Drawable
{
public:
    Cube(OpenGLContext* context) : Drawable(context){}
    virtual ~Cube(){}
    void create() override;
};
