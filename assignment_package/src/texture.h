#pragma once

#include <openglcontext.h>
#include <glm_includes.h>
#include <memory>

class Texture
{
public:
    Texture(OpenGLContext* context);
    ~Texture();

    void create();
    void load(int texSlot);
    void bind(int texSlot);

private:
    OpenGLContext* context;
    GLuint m_textureHandle;
    std::shared_ptr<QImage> m_textureImage;
};
