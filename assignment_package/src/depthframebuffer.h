#ifndef DEPTHFRAMEBUFFER_H
#define DEPTHFRAMEBUFFER_H

#include "framebuffer.h"

class DepthFrameBuffer : public FrameBuffer
{
public:
    GLuint m_depthTexture;
    DepthFrameBuffer(OpenGLContext *context, unsigned int width, unsigned int height, unsigned int devicePixelRatio);
    void create();
    void destroy();
};

#endif // DEPTHFRAMEBUFFER_H
