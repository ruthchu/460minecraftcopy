#ifndef MYGL_H
#define MYGL_H

#include "openglcontext.h"
#include "shaderprogram.h"
#include "scene/worldaxes.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/player.h"
#include "texture.h"
#include "framebuffer.h"
#include "postprocessingshader.h"
#include "scene/quad.h"
#include "depthframebuffer.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    WorldAxes m_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    Texture m_texture; // A texture to be used in the lambert shader

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Terrain m_terrain; // All of the Chunks that currently comprise the world.
    Player m_player; // The entity controlled by the user. Contains a camera to display what it sees as well.
    InputBundle m_inputs; // A collection of variables to be updated in keyPressEvent, mouseMoveEvent, mousePressEvent, etc.

    QTimer m_timer; // Timer linked to tick(). Fires approximately 60 times per second.

    double m_currTime; // Current time used to compute dT for player movement
    int m_timeSinceStart; // Time passed to UVs to warp LAVA and WATER UV coords

    FrameBuffer m_framebuffer; // Frame buffer for post processing

    PostProcessingShader m_progTint; // A post processing shader program that handels water and lava tinting
    PostProcessingShader m_progNoOp; // A post processing shader program that does nothing

    DepthThroughShader m_progDepthThrough; // A post shader program to see depth values

    PostProcessingShader m_progShandow; // A post processing shader program that does nothing

    ShaderProgram m_progSky; // A screen-space shader for creating the sky background
    float time;

    Quad quad;

    DepthFrameBuffer m_depthFrameBuffer;

    void moveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    void sendPlayerDataToGUI() const;

    void performTerrainPostprocessRenderPass();

    void preformLightPerspectivePass();

    void preformPlayerPerspectivePass();

    void prepareViewportForFBO();

    int playerIsInLiquid();

public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    // Called once when MyGL is initialized.
    // Once this is called, all OpenGL function
    // invocations are valid (before this, they
    // will cause segfaults)
    void initializeGL();
    // Called whenever MyGL is resized.
    void resizeGL(int w, int h);
    // Called whenever MyGL::update() is called.
    // In the base code, update() is called from tick().
    void paintGL();

    // Called from paintGL().
    // Calls Terrain::draw().
    void renderTerrain(ShaderProgram *prog);

protected:
    // Automatically invoked when the user
    // presses a key on the keyboard
    void keyPressEvent(QKeyEvent *e);
    // Automatically invoked when the user
    // releases a key on the keyboard
    void keyReleaseEvent(QKeyEvent *e);
    // Automatically invoked when the user
    // moves the mouse
    void mouseMoveEvent(QMouseEvent *e);
    // Automatically invoked when the user
    // presses a mouse button
    void mousePressEvent(QMouseEvent *e);

private slots:
    void tick(); // Slot that gets called ~60 times per second by m_timer firing.

signals:
    void sig_sendPlayerPos(QString) const;
    void sig_sendPlayerVel(QString) const;
    void sig_sendPlayerAcc(QString) const;
    void sig_sendPlayerLook(QString) const;
    void sig_sendPlayerChunk(QString) const;
    void sig_sendPlayerTerrainZone(QString) const;
};


#endif // MYGL_H
