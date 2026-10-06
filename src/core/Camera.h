#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/**
 * @brief Opciones de movimiento de la cámara libre.
 */
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

/**
 * @brief Valores por defecto de configuración de la cámara.
 */
namespace CameraDefaults {
    constexpr float YAW         = -90.0f;
    constexpr float PITCH       =   0.0f;
    constexpr float SPEED       =   2.5f;
    constexpr float SENSITIVITY =   0.1f;
    constexpr float FOV         =  45.0f; 
    constexpr float NEAR_PLANE  =   0.1f;
    constexpr float FAR_PLANE   = 100.0f;
}

/**
 * @brief Cámara libre en primera persona (Euler Angles + LookAt).
 * 
 * Calcula las matrices de Vista (View) y Proyección (Projection) para el pipeline gráfico.
 * Procesa entradas de teclado (WASD / Elevación), desplazamiento del ratón (Yaw / Pitch)
 * y rueda de scroll (Zoom / FOV).
 */
class Camera {
public:
    
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    
    float yaw;
    float pitch;

    
    float movementSpeed;
    float mouseSensitivity;
    float fov;
    float nearPlane;
    float farPlane;

    /**
     * @brief Constructor con vectores de posición y orientación.
     */
    Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw = CameraDefaults::YAW,
           float startPitch = CameraDefaults::PITCH);

    /**
     * @brief Constructor con coordenadas escalares.
     */
    Camera(float posX, float posY, float posZ,
           float upX, float upY, float upZ,
           float startYaw, float startPitch);

    /**
     * @brief Retorna la matriz de vista calculada mediante glm::lookAt.
     */
    glm::mat4 getViewMatrix() const;

    /**
     * @brief Retorna la matriz de proyección en perspectiva según el aspecto de la ventana.
     * @param aspectRatio Relación de aspecto (ancho / alto).
     */
    glm::mat4 getProjectionMatrix(float aspectRatio) const;

    /**
     * @brief Procesa el desplazamiento traslacional (WASD / Espacio / Shift).
     * @param direction Dirección de movimiento.
     * @param deltaTime Tiempo transcurrido entre cuadros (en segundos) para garantizar velocidad constante.
     */
    void processKeyboard(CameraMovement direction, float deltaTime);

    /**
     * @brief Procesa el movimiento angular del ratón (Yaw y Pitch).
     * @param xOffset Desplazamiento horizontal del cursor.
     * @param yOffset Desplazamiento vertical del cursor.
     * @param constrainPitch Si es true, restringe el cabeceo a [-89.0°, 89.0°] para evitar volteos.
     */
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);

    /**
     * @brief Procesa el scroll del ratón para modificar el campo de visión (Zoom).
     * @param yOffset Desplazamiento vertical de la rueda de desplazamiento.
     */
    void processMouseScroll(float yOffset);

    /**
     * @brief Restablece la cámara a su posición y ángulos iniciales por defecto.
     */
    void reset();

private:
    /**
     * @brief Recalcula los vectores front, right y up a partir de los ángulos de Euler actuales.
     */
    void updateCameraVectors();
};
