#include "camera.h"
#include "glm_includes.h"
#include <iostream>

Camera::Camera(glm::vec3 pos)
    : Camera(400, 400, pos)
{}

Camera::Camera(unsigned int w, unsigned int h, glm::vec3 pos)
    : Entity(pos), m_fovy(45), m_width(w), m_height(h),
      m_near_clip(0.1f), m_far_clip(1000.f), m_aspect(w / static_cast<float>(h))
{}

Camera::Camera(const Camera &c)
    : Entity(c),
      m_fovy(c.m_fovy),
      m_width(c.m_width),
      m_height(c.m_height),
      m_near_clip(c.m_near_clip),
      m_far_clip(c.m_far_clip),
      m_aspect(c.m_aspect)
{}


void Camera::setWidthHeight(unsigned int w, unsigned int h) {
    m_width = w;
    m_height = h;
    m_aspect = w / static_cast<float>(h);
}


void Camera::tick(float dT, InputBundle &input) {
    // Do nothing
}

glm::mat4 Camera::getViewProj() const {
    return glm::perspective(glm::radians(m_fovy), m_aspect, m_near_clip, m_far_clip) * glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::mat4 Camera::getView() const {
    std::cout << "m_pos: " << m_position.x << ", " << m_position.y << " ," << m_position.z << std::endl;
    glm::vec3 a = m_position + m_forward;
    std::cout << "m_pos + forward: " << a.x << ", " << a.y << " ," << a.z << std::endl;
    a = m_up;
    std::cout << "m_up: " << a.x << ", " << a.y << " ," << a.z << std::endl;
    std::cout << "--------------------------" << std::endl;
    return glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::vec3 Camera::getLookVec() const {
    return m_forward;
}
