#ifndef POSTPROCESSINGSHADER_H
#define POSTPROCESSINGSHADER_H

#include <openglcontext.h>
#include <glm_includes.h>
#include <glm/glm.hpp>
// For glm::translate, glm::rotate, and glm::scale.
#    include <glm/gtc/matrix_transform.hpp>
// For glm::to_string.
#    include <glm/gtx/string_cast.hpp>
// For glm::value_ptr.
#    include <glm/gtc/type_ptr.hpp>

#include "drawable.h"
#include "texture.h"

class PostProcessingShader
{
public:
    GLuint vertShader; // A handle for the vertex shader stored in this shader program
    GLuint fragShader; // A handle for the fragment shader stored in this shader program
    GLuint prog;       // A handle for the linked shader program stored in this class

    int unifSampler2D; // A handle to the "uniform" sampler2D that will be used to read the texture containing the scene render
    int unifTime; // A handle for the "uniform" float representing time in the shader

    int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
    int attrUV; // A handle for the "in" vec2 representing the UV coordinates in the vertex shader

    int unifDimensions; // A handle to the "uniform" ivec2 that stores the width and height of the texture being rendered

    int unifDepthMatrixID; // A handle to the "unifrom" mat that projects us from world to light pov
public:
    PostProcessingShader(OpenGLContext* context);
    // draw given object to screen using shaderprogram shaders
    ~PostProcessingShader();
    void create(const char *vertfile, const char *fragfile);
    void useMe();
    void draw(Drawable &d, int textureSlot);

    char* textFileRead(const char*);
    QString qTextFileRead(const char *fileName);

    void printLinkInfoLog(int prog);
    void printShaderInfoLog(int shader);

    void setDimensions(glm::ivec2 dims);

    void setTime(int t);

    void setModelMatrix(const glm::mat4 &model);

    void setDepthMVP(const glm::vec3 inverseLightRay);
protected:
    OpenGLContext* context;
};

#endif // POSTPROCESSINGSHADER_H
