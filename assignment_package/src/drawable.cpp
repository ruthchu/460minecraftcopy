#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(-1), m_count_t(-1),
      m_bufIdx(), m_bufIdxTransparent(),
      m_bufPos(), m_bufNor(), m_bufCol(),
      m_buffAllOpaque(), m_buffAllTransparent(),
      m_idxGenerated(false), m_idxTransparentGenerated(false),
      m_posGenerated(false), m_norGenerated(false), m_colGenerated(false),
      m_allGeneratedOpaque(false), m_allGeneratedTransparent(false),
      mp_context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    mp_context->glDeleteBuffers(1, &m_bufIdx);
    mp_context->glDeleteBuffers(1, &m_bufIdxTransparent);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);
    mp_context->glDeleteBuffers(1, &m_buffAllOpaque);
    mp_context->glDeleteBuffers(1, &m_buffAllTransparent);
    m_idxGenerated = m_idxTransparentGenerated = m_posGenerated = m_norGenerated = m_colGenerated = m_allGeneratedOpaque = m_allGeneratedTransparent = false;
    m_count = -1;
    m_count_t = -1;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCountOpaque()
{
    return m_count;
}

int Drawable::elemCountTransparent()
{
    return m_count_t;
}

void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

void Drawable::generateIdxTransparent()
{
    m_idxTransparentGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdxTransparent);
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generateAllOpaque()
{
    m_allGeneratedOpaque = true;
    // Create a VBO on our GPU and store its handle in bufAll
    mp_context->glGenBuffers(1, &m_buffAllOpaque);
}

void Drawable::generateAllTransparent()
{
    m_allGeneratedTransparent = true;
    // Create a VBO on our GPU and store its handle in bufAll
    mp_context->glGenBuffers(1, &m_buffAllTransparent);
}


bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxGenerated;
}

bool Drawable::bindIdxTransparent()
{
    if(m_idxTransparentGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTransparent);
    }
    return m_idxTransparentGenerated;
}


bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindAllOpaque()
{
    if(m_allGeneratedOpaque) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_buffAllOpaque);
    }
    return m_allGeneratedOpaque;
}

bool Drawable::bindAllTransparent()
{
    if(m_allGeneratedTransparent) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_buffAllTransparent);
    }
    return m_allGeneratedTransparent;
}

