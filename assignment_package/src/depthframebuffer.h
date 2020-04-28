#ifndef DEPTHFRAMEBUFFER_H
#define DEPTHFRAMEBUFFER_H

#include "openglcontext.h"
#include "glm_includes.h"
#include <smartpointerhelp.h>

class DepthFrameBuffer
{
private:
    OpenGLContext *mp_context;
    unsigned int m_width, m_height, m_devicePixelRatio;
    bool m_created;

    unsigned int m_textureSlot;
public:
    GLuint m_frameBuffer;
    GLuint m_outputTexture;

    DepthFrameBuffer(OpenGLContext *context, unsigned int width, unsigned int height, unsigned int devicePixelRatio);

    void resize(unsigned int width, unsigned int height, unsigned int devicePixelRatio);

    void create();

    void destroy();

    void bindFrameBuffer();

    void bindToTextureSlot(unsigned int slot);
    unsigned int getTextureSlot() const;
};

#endif // DEPTHFRAMEBUFFER_H
