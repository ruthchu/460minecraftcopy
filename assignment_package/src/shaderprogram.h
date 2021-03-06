#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <openglcontext.h>
#include <glm_includes.h>
#include <glm/glm.hpp>

#include "drawable.h"


class ShaderProgram
{
public:
    GLuint vertShader; // A handle for the vertex shader stored in this shader program
    GLuint fragShader; // A handle for the fragment shader stored in this shader program
    GLuint prog;       // A handle for the linked shader program stored in this class

    int attrPos; // A handle for the "in" vec4 representing vertex position in the vertex shader
    int attrNor; // A handle for the "in" vec4 representing vertex normal in the vertex shader
    int attrCol; // A handle for the "in" vec4 representing vertex color in the vertex shader

    int unifModel; // A handle for the "uniform" mat4 representing model matrix in the vertex shader
    int unifModelInvTr; // A handle for the "uniform" mat4 representing inverse transpose of the model matrix in the vertex shader
    int unifViewProj; // A handle for the "uniform" mat4 representing combined projection and view matrices in the vertex shader
    int unifColor; // A handle for the "uniform" vec4 representing color of geometry in the vertex shader

    int unifSampler2D; // A handle to the "uniform" sampler2D that will be used to read the texture containing the scene render
    int unifSampler2DShadow;

    int unifTime; // A handle for the "uniform" float representing time in the shader
    int unifView; // A handle for the "uniform" mat4 to bring from

    int unifDepthMatrixID; // A handle to the "unifrom" mat that projects us from world to light pov
    // Sky
    int unifDimensions;
    int unifEye;

    int unifLightProj;

public:
    ShaderProgram(OpenGLContext* context);
    // Sets up the requisite GL data and shaders from the given .glsl files
    void create(const char *vertfile, const char *fragfile);
    // Tells our OpenGL context to use this shader to draw things
    void useMe();
    // Pass the given model matrix to this shader on the GPU
    void setModelMatrix(const glm::mat4 &model);
    // Pass the given Projection * View matrix to this shader on the GPU
    void setViewMatrix(const glm::mat4 &v);
    void setViewProjMatrix(const glm::mat4 &vp);
    // Pass the given color to this shader on the GPU
    void setGeometryColor(glm::vec4 color);
    // Pass a texture to this shader on the GPU
    void setTextureSampler2D(int textureSlot);
    void setTextureSampler2DShadow(int textureSlot);
    // Pass a time variable to this shader on the GPU
    void setTime(int t);
    // Draw the given object to our screen using this ShaderProgram's shaders
    void drawQuad(Drawable &d);
    virtual void drawOpaque(Drawable &d);
    virtual void drawTransparent(Drawable &d);
    // Utility function used in create()
    char* textFileRead(const char*);
    // Utility function that prints any shader compilation errors to the console
    void printShaderInfoLog(int shader);
    // Utility function that prints any shader linking errors to the console
    void printLinkInfoLog(int prog);

    void setDepthMVP(const glm::vec3 light);
    void setDepthMVP(const glm::mat4 mat);

    void setLightProj(const glm::mat4 &v);

    void setDimensions(glm::ivec2 dims);

    QString qTextFileRead(const char*);

protected:
    OpenGLContext* context;   // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                            // we need to pass our OpenGL context to the Drawable in order to call GL functions
                            // from within this class.
};

class DepthThroughShader : public ShaderProgram
{
public:
    DepthThroughShader(OpenGLContext* context);
    ~DepthThroughShader();

    void drawOpaque(Drawable &d) override;
    void drawTransparent(Drawable &d) override;
};

#endif // SHADERPROGRAM_H
