#include "quad.h"

Quad::Quad(OpenGLContext *context) : Drawable(context)
{}

void Quad::create()
{
    idx.clear();
    data.clear();

    GLuint idxarr[] {0, 1, 2, 0, 2, 3};

    for (GLuint i : idxarr) {
        this->idx.push_back(i);
    }

    glm::vec4 posUVarr[] = {glm::vec4(-1.f, -1.f, 0.99f, 1.f), glm::vec4(0.f, 0.f, 0.f, 0.f),
                           glm::vec4(1.f, -1.f, 0.99f, 1.f), glm::vec4(1.f, 0.f, 0.f, 0.f),
                           glm::vec4(1.f, 1.f, 0.99f, 1.f), glm::vec4(1.f, 1.f, 0.f, 0.f),
                           glm::vec4(-1.f, 1.f, 0.99f, 1.f), glm::vec4(0.f, 1.f, 0.f, 0.f)};

    for (glm::vec4 d : posUVarr) {
        this->data.push_back(d);
    }

//    glm::vec2 vert_UV[4] {glm::vec2(0.f, 0.f),
//                          glm::vec2(1.f, 0.f),
//                          glm::vec2(1.f, 1.f),
//                          glm::vec2(0.f, 1.f)};
}

void Quad::bufferVBOdata() {
    m_count = idx.size();
    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    bindIdx();
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // CYL_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), this->idx.data(), GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generateAllOpaque();
    bindAllOpaque();
    mp_context->glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec4), this->data.data(), GL_STATIC_DRAW);
}
