#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_texture(this),
      m_terrain(this), m_player(glm::vec3(48.f, 170.f, 48.f), m_terrain),
      m_currTime(QDateTime::currentMSecsSinceEpoch())
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // Create and load the appropriate texture
    m_texture.create();
    m_texture.load(0);

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    // Move the cursor to the middle of the screen
    moveMouseToCenter();
    m_inputs.prevMouseX = width() / 2;
    m_inputs.prevMouseY = height() / 2;
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    // check if new terrain zone chunks need to be created and populated
    float dT = (QDateTime::currentMSecsSinceEpoch() - m_currTime) / 1000.f;
    m_player.tick(dT, m_inputs);
    m_currTime = QDateTime::currentMSecsSinceEpoch();

    m_terrain.expandTerrainBasedOnPlayer(m_player.mcr_position);
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    renderTerrain();

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    int renderRadius = 1;
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 centerTerrain = m_terrain.getTerrainAt(pPos[0], pPos[1]);

    // Bind the texture
    m_texture.bind(0);

    int xmin = centerTerrain[0] - BLOCK_LENGTH_IN_TERRAIN * renderRadius /*- BLOCK_LENGTH_IN_TERRAIN*/;//16 * (xFloor - range);
    int xmax = centerTerrain[0] + BLOCK_LENGTH_IN_TERRAIN * renderRadius + BLOCK_LENGTH_IN_TERRAIN; //16 * (xFloor + range);
    int zmin = centerTerrain[1] - BLOCK_LENGTH_IN_TERRAIN * renderRadius - BLOCK_LENGTH_IN_TERRAIN;//16 * (zFloor - range);
    int zmax = centerTerrain[1] + BLOCK_LENGTH_IN_TERRAIN * renderRadius /*+ BLOCK_LENGTH_IN_TERRAIN*/;//16 * (zFloor + range);
    m_terrain.draw(xmin, xmax, zmin, zmax, &m_progLambert);
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_F) {
       m_player.m_flightOn = !m_player.m_flightOn;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // Moves camera
    m_inputs.mouseX = (m_inputs.prevMouseX - e->x()) / 2.f;
    m_inputs.mouseY = (m_inputs.prevMouseY - e->y()) / 2.f;
    moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // Place or remove blocks
    glm::ivec3 hitBlock;
    float outLen = 0.f;
    const glm::vec3 mid = m_player.mcr_camera.mcr_position;
    glm::vec3 ray = glm::normalize(m_player.mcr_camera.getLookVec());
    if (e->button() == Qt::LeftButton) {
        // Remove block
        if (m_player.gridMarch(mid, ray * 3.f, m_terrain, &outLen, &hitBlock)) {
            m_terrain.setBlockAt(hitBlock.x, hitBlock.y, hitBlock.z, EMPTY);
        }
    } else if (e->button() == Qt::RightButton) {
        if (m_player.gridMarch(mid, ray * 3.f, m_terrain, &outLen, &hitBlock)) {
            // Determine where to place block based on ray direction
            // Find the middle of the block
            glm::vec3 blockMid = {hitBlock.x + .5f,
                                  hitBlock.y + .5f,
                                  hitBlock.z + .5f};
            // Find where the ray collides with the block
            glm::vec3 rayHit = m_player.mcr_camera.mcr_position + ray * outLen;
            // Find the difference between the two points and set the block
            // adjacent to the face that has the largest distance from the center
            glm::vec3 diff = rayHit - blockMid;
            glm::ivec3 mod = glm::ivec3(0);
            if (abs(diff.x) > abs(diff.y) && abs(diff.x) > abs(diff.z)) {
                if (diff.x > 0) {
                    mod.x = 1;
                } else {
                    mod.x = -1;
                }
            } else if (abs(diff.y) > abs(diff.x) && abs(diff.y) > abs(diff.z)) {
                if (diff.y > 0) {
                    mod.y = 1;
                } else {
                    mod.y = -1;
                }
            } else if (abs(diff.z) > abs(diff.x) && abs(diff.z) > abs(diff.y)) {
                if (diff.z > 0) {
                    mod.z = 1;
                } else {
                    mod.z = -1;
                }
            }
            m_terrain.setBlockAt(hitBlock.x + mod.x, hitBlock.y + mod.y,
                                 hitBlock.z + mod.z, DIRT);
        }
    }
}
