#ifndef QUAD_H
#define QUAD_H

#include "drawable.h"
#    include <glm/glm.hpp>
// For glm::translate, glm::rotate, and glm::scale.
#    include <glm/gtc/matrix_transform.hpp>
// For glm::to_string.
#    include <glm/gtx/string_cast.hpp>
// For glm::value_ptr.
#    include <glm/gtc/type_ptr.hpp>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Quad : public Drawable
{
private:
    std::vector<GLuint> idx;
    // pos and uv data
    std::vector<glm::vec4> data;
public:
    Quad(OpenGLContext* context);
    virtual void create();
    void createCube();
    void bufferVBOdata();
};

#endif // QUAD_H
