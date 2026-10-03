#include "core/Camera.h"

#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
    : position(startPosition),
      front(glm::vec3(0.0f, 0.0f, -1.0f)),
      worldUp(startUp),
      yaw(startYaw),
      pitch(startPitch),
      movementSpeed(CameraDefaults::SPEED),
      mouseSensitivity(CameraDefaults::SENSITIVITY),
      fov(CameraDefaults::FOV),
      nearPlane(CameraDefaults::NEAR_PLANE),
      farPlane(CameraDefaults::FAR_PLANE) {
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ,
               float upX, float upY, float upZ,
               float startYaw, float startPitch)
    : position(glm::vec3(posX, posY, posZ)),
      front(glm::vec3(0.0f, 0.0f, -1.0f)),
      worldUp(glm::vec3(upX, upY, upZ)),
      yaw(startYaw),
      pitch(startPitch),
      movementSpeed(CameraDefaults::SPEED),
      mouseSensitivity(CameraDefaults::SENSITIVITY),
      fov(CameraDefaults::FOV),
      nearPlane(CameraDefaults::NEAR_PLANE),
      farPlane(CameraDefaults::FAR_PLANE) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    // glm::lookAt(eye, center, up)
    // eye: posición de la cámara en el mundo
    // center: punto hacia el que apunta (posición + vector de vista)
    // up: vector perpendicular que define la orientación superior
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    // Proyección en perspectiva usando FOV vertical en radianes
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    const float velocity = movementSpeed * deltaTime;

    switch (direction) {
        case CameraMovement::FORWARD:
            position += front * velocity;
            break;
        case CameraMovement::BACKWARD:
            position -= front * velocity;
            break;
        case CameraMovement::LEFT:
            position -= right * velocity;
            break;
        case CameraMovement::RIGHT:
            position += right * velocity;
            break;
        case CameraMovement::UP:
            position += worldUp * velocity;
            break;
        case CameraMovement::DOWN:
            position -= worldUp * velocity;
            break;
    }
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    // Acotar el cabeceo (pitch) para evitar el bloqueo del cardán (Gimbal Lock) o volteo de cámara
    if (constrainPitch) {
        pitch = std::clamp(pitch, -89.0f, 89.0f);
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yOffset) {
    fov -= yOffset;
    // Acotar el zoom / campo de visión entre 1.0° y 45.0°
    fov = std::clamp(fov, 1.0f, 45.0f);
}

void Camera::reset() {
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = CameraDefaults::YAW;
    pitch = CameraDefaults::PITCH;
    fov = CameraDefaults::FOV;
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // 1. Calcular el nuevo vector de dirección Front a partir de trigonometría esférica
    glm::vec3 newFront;
    const float yawRad = glm::radians(yaw);
    const float pitchRad = glm::radians(pitch);

    newFront.x = std::cos(yawRad) * std::cos(pitchRad);
    newFront.y = std::sin(pitchRad);
    newFront.z = std::sin(yawRad) * std::cos(pitchRad);
    front = glm::normalize(newFront);

    // 2. Recalcular los vectores Right y Up usando productos cruz normalizados
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
