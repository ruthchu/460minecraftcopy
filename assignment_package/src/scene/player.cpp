#include "player.h"
#include <QString>
#include <iostream>

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain), m_phi(0.f),
      mcr_camera(m_camera), m_flightOn(true)
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input, dT);
    computePhysics(dT, mcr_terrain);
}

void Player::processInputs(InputBundle &inputs, float dT) {
    // Rotate the local axis' based on mouse input
    float mod = 1.f / dT;
    rotateOnUpGlobal(inputs.mouseX / 5.f * mod);
    if (m_phi < 90.f && m_phi > -90.f) {
        rotateOnRightLocal
            (glm::clamp(inputs.mouseY / 5.f * mod, -89.99f - m_phi, 89.99f - m_phi));
    }
    m_phi = glm::clamp(m_phi + inputs.mouseY, -89.99f, 89.99f);
    inputs.mouseX = 0.f;
    inputs.mouseY = 0.f;
    m_acceleration = {0.f, 0.f, 0.f};
    float accel = mod * 3.f;
    // Movement in flight mode
    if (m_flightOn) {
        if (inputs.wPressed == true) {
            // Accelerate positively along forward vector
            m_acceleration += accel * 1.1f * m_forward;
        }
        if (inputs.aPressed == true) {
            // Accelerate negatively along right vector
            m_acceleration += -accel * m_right;
        }
        if (inputs.sPressed == true) {
            // Accelerate negatively along forward vector
            m_acceleration += -accel * m_forward;
        }
        if (inputs.dPressed == true) {
            // Accelerate positively along right vector
            m_acceleration += accel * m_right;
        }
        if (inputs.qPressed == true) {
            // Accelerate negatively along the up vector
            m_acceleration += -accel * glm::vec3(0.f, 1.f, 0.f);
        }
        if (inputs.ePressed == true) {
            // Accelerate positively along the up vector
            m_acceleration += accel * glm::vec3(0.f, 1.f, 0.f);
        }
    } else if (!m_flightOn) {
        m_acceleration.y = -accel * 2.f;
        // Movement in non-flight mode
        glm::vec3 flatForward =
                glm::normalize(glm::vec3(m_forward.x, 0.f, m_forward.z));
        if (inputs.wPressed == true) {
            // Accelerate positively along projected forward vector
            m_acceleration += accel * 1.1f * flatForward;
        }
        if (inputs.sPressed == true) {
            // Accelerate negatively along projected forward vector
            m_acceleration += -accel * flatForward;
        }
        glm::vec3 flatRight =
                glm::normalize(glm::vec3(m_right.x, 0.f, m_right.z));
        if (inputs.aPressed == true) {
            // Accelerate negatively along projected right vector
            m_acceleration += -accel * flatRight;
        }
        if (inputs.dPressed == true) {
            // Accelerate positively along projected right vector
            m_acceleration += accel * flatRight;
        }
        if (inputs.spacePressed == true) {
            if (m_velocity.y < accel * dT * dT * .8f) {
                m_velocity.y += 3.f * accel;
            }
            inputs.spacePressed = false;
        }
    }
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    // Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    m_velocity = m_velocity * .5f + dT * m_acceleration;
    glm::vec3 move = m_velocity * dT;
    if (!m_flightOn) {
        float xDist;
        float yDist;
        float zDist;
        glm::ivec3 blockHit;
        glm::vec3 moveX = {move.x, 0.f, 0.f};
        glm::vec3 moveY = {0.f, move.y, 0.f};
        glm::vec3 moveZ = {0.f, 0.f, move.z};

        // For each point on the player, check collisions by grid marching along
        // each axis
        for (float y = 0.f; y <= 2.f; y += 1.f) {
            for (float x = -.5f; x <= .5f; x += 1.f) {
                for (float z = -.5f; z <= .5f; z += 1.f) {
                    glm::vec3 origin = {m_position.x + x, m_position.y + y,
                                        m_position.z + z};
                    if (gridMarch(origin, moveX, terrain, &xDist, &blockHit)) {
                        if (xDist < std::abs(move.x)) {
                            if (move.x < 0.f) {
                                move.x = -xDist + .0001f;
                            } else {
                                move.x = xDist - .0001f;
                            }
                        }
                    }

                    if (gridMarch(origin, moveY, terrain, &yDist, &blockHit)) {
                        if (yDist < std::abs(move.y)) {
                            if (move.y < 0.f) {
                                move.y = -yDist + .0001f;
                            } else {
                                move.y = yDist - .0001f;
                            }
                        }
                    }
                    if (gridMarch(origin, moveZ, terrain, &zDist, &blockHit)) {
                        if (zDist < std::abs(move.z)) {
                            if (move.z < 0.f) {
                                move.z = -zDist + .0001f;
                            } else {
                                move.z = zDist - .0001f;
                            }
                        }
                    }
                }
            }
        }
    }
    moveAlongVector(move);
}

bool Player::gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection,
        const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit) {
    float maxLen = glm::length(rayDirection); // Farthest we search
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    rayDirection = glm::normalize(rayDirection); // Now all t values represent world dist.

    float curr_t = 0.f;
    while(curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1; // Track axis for which t is smallest
        for(int i = 0; i < 3; ++i) { // Iterate over the three axes
            if(rayDirection[i] != 0) { // Is ray parallel to axis i?
                float offset = glm::max(0.f, glm::sign(rayDirection[i]));
                // If the player is *exactly* on an interface then
                // they'll never move if they're looking in a negative direction
                if(currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen); // Clamp to max len to avoid super out of bounds errors
                if(axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if(interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t;
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0,0,0);
        // Sets it to 0 if sign is +, -1 if sign is -
        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;
        // If currCell contains something other than EMPTY, return
        // curr_t
        BlockType cellType = terrain.getBlockAt(currCell.x, currCell.y, currCell.z);
        if(cellType != EMPTY) {
            *out_blockHit = currCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
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
