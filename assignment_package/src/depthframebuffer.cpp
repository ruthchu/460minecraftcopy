#include "depthframebuffer.h"
#include <iostream>

DepthFrameBuffer::DepthFrameBuffer(OpenGLContext *context, unsigned int width, unsigned int height, unsigned int devicePixelRatio)
    : mp_context(context), m_frameBuffer(-1),
      m_outputTexture(-1), m_width(width), m_height(height), m_devicePixelRatio(devicePixelRatio), m_created(false)
{}

void DepthFrameBuffer::resize(unsigned int width, unsigned int height, unsigned int devicePixelRatio)
{
    m_width = width;
    m_height = height;
#ifdef MAC
    m_width = 2 * width;
    m_height = 2 * height;
#endif
    m_devicePixelRatio = devicePixelRatio;
}

void DepthFrameBuffer::create()
{
    mp_context->glGenFramebuffers(1, &m_frameBuffer);
    mp_context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

    mp_context->glGenTextures(1, &m_outputTexture);
    mp_context->glBindTexture(GL_TEXTURE_2D, m_outputTexture);

//    mp_context->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    mp_context->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    mp_context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    mp_context->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_outputTexture, 0);

    mp_context->glDrawBuffer(GL_NONE); // no color drawn
    mp_context->glReadBuffer(GL_NONE); // no color drawn

    m_created = true;
    if(mp_context->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        m_created = false;
        std::cout << "Depth Frame buffer did not initialize correctly..." << std::endl;
        mp_context->printGLErrorLog();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFrameBuffer::destroy() {
    if(m_created) {
        m_created = false;
        mp_context->glDeleteFramebuffers(1, &m_frameBuffer);
        mp_context->glDeleteTextures(1, &m_outputTexture);
    }
}

void DepthFrameBuffer::bindFrameBuffer() {
    mp_context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
}

void DepthFrameBuffer::bindToTextureSlot(unsigned int slot) {
    m_textureSlot = slot;
    mp_context->glActiveTexture(GL_TEXTURE0 + slot);
    mp_context->glBindTexture(GL_TEXTURE_2D, m_outputTexture);
}

unsigned int DepthFrameBuffer::getTextureSlot() const {
    return m_textureSlot;
}


