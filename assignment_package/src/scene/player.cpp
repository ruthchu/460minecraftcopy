#include "player.h"
#include <QString>
#include <iostream>
#include <iomanip>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
      mcr_camera(m_camera), m_flightOn(true)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.

    // Rotate the local axis' based on mouse input
    rotateOnUpGlobal(inputs.mouseX);
    if (inputs.phi < 90.f && inputs.phi > -90.f) {
        rotateOnRightLocal
            (glm::clamp(-90.f - inputs.phi, 90.f - inputs.phi, inputs.mouseY));
    }
    inputs.phi = glm::clamp(-90.f, 90.f, inputs.phi + inputs.mouseY);
    inputs.mouseX = 0.f;
    inputs.mouseY = 0.f;
    m_acceleration = glm::vec3(0.f, 0.f, 0.f);
    // Movement in flight mode
    if (m_flightOn) {
        if (inputs.wPressed == true) {
            // Accelerate positively along forward vector
            m_acceleration += 1.1f * m_forward;
        }
        if (inputs.aPressed == true) {
            // Accelerate negatively along right vector
            m_acceleration += -1.1f * m_right;
        }
        if (inputs.sPressed == true) {
            // Accelerate negatively along forward vector
            m_acceleration += -1.1f * m_forward;
        }
        if (inputs.dPressed == true) {
            // Accelerate positively along right vector
            m_acceleration += 1.1f * m_right;
        }
        if (inputs.qPressed == true) {
            // Accelerate negatively along the up vector
            m_acceleration += -1.1f * m_up;
        }
        if (inputs.ePressed == true) {
            // Accelerate positively along the up vector
            m_acceleration += 1.1f * m_up;
        }
    } else {
        // Movement in non-flight mode
        glm::vec3 flatForward =
                glm::normalize(glm::vec3(m_forward.x, 0.f, m_forward.z));
        if (inputs.wPressed == true) {
            // Accelerate positively along projected forward vector
            m_acceleration = 1.1f * flatForward;
        }
        if (inputs.sPressed == true) {
            // Accelerate negatively along projected forward vector
            m_acceleration = -1.1f * flatForward;
        }
        glm::vec3 flatRight =
                glm::normalize(glm::vec3(m_right.x, 0.f, m_right.z));
        if (inputs.aPressed == true) {
            // Accelerate negatively along projected right vector
            m_acceleration = -1.1f * flatRight;
        }
        if (inputs.dPressed == true) {
            // Accelerate positively along projected right vector
            m_acceleration = 1.1f * flatRight;
        }
        if (inputs.spacePressed == true) {
            m_velocity.y = 5.f;
            inputs.spacePressed = false;
        }
    }
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.

    m_velocity = m_velocity * .5f + dT * m_acceleration;
    moveAlongVector(m_velocity * dT);
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
