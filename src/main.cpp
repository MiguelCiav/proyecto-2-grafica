#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/Shader.h"
#include "core/Camera.h"
#include "graphics/Mesh.h"
#include "ui/EditorUI.h"

#include <iostream>
#include <vector>

// Configuración inicial de la ventana
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

// Instancia global de la cámara (Dev A)
Camera camera(glm::vec3(0.0f, 1.0f, 4.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Temporización (frametime independiente)
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Prototipos de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

int main() {
    // 1. Inicialización y configuración de GLFW
    if (!glfwInit()) {
        std::cerr << "[ERROR::GLFW] Falló la inicialización de GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 2. Creación de la ventana GLFW (1280x720)
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto #2 - Computación Gráfica (UCV)", nullptr, nullptr);
    if (!window) {
        std::cerr << "[ERROR::GLFW] Falló la creación de la ventana GLFW." << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Registro de callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Habilitar V-Sync para estabilidad
    glfwSwapInterval(1);

    // 3. Inicialización de GLAD (debe ejecutarse tras glfwMakeContextCurrent)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[ERROR::GLAD] Falló la inicialización de punteros de OpenGL con GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Configuración inicial del viewport y estados de OpenGL
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // 4. Inicialización de Dear ImGui (Dev B)
    EditorUI editorUI;
    editorUI.init(window);

    // 5. Carga y compilación de shaders base (Dev A)
    Shader defaultShader("assets/shaders/default.vert", "assets/shaders/default.frag");

    // 6. Creación de geometría de prueba: Cubo 3D con normales y UVs (Dev B - Mesh)
    std::vector<Vertex> cubeVertices = {
        // Cara frontal (Z+)
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f,  1.0f}, {0.0f, 1.0f} },

        // Cara trasera (Z-)
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} },
        { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} },

        // Cara superior (Y+)
        { {-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f, 0.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f, 0.0f}, {0.0f, 1.0f} },

        // Cara inferior (Y-)
        { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },

        // Cara derecha (X+)
        { { 0.5f, -0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },

        // Cara izquierda (X-)
        { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} }
    };

    std::vector<unsigned int> cubeIndices = {
         0,  1,  2,   2,  3,  0, // Frontal
         4,  5,  6,   6,  7,  4, // Trasera
         8,  9, 10,  10, 11,  8, // Superior
        12, 13, 14,  14, 15, 12, // Inferior
        16, 17, 18,  18, 19, 16, // Derecha
        20, 21, 22,  22, 23, 20  // Izquierda
    };

    Mesh testMesh(cubeVertices, cubeIndices);

    // 7. Bucle Principal de Renderizado
    while (!glfwWindowShouldClose(window)) {
        // Cálculo del tiempo por fotograma (deltaTime)
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Procesar entradas de teclado
        processInput(window);

        // Limpieza de los buffers de color y profundidad
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activación del shader y paso de matrices MVP
        defaultShader.use();

        float aspect = (SCR_HEIGHT > 0) ? (static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT)) : 1.0f;
        glm::mat4 projection = camera.getProjectionMatrix(aspect);
        glm::mat4 view = camera.getViewMatrix();

        // Rotación suave del cubo de prueba para verificar profundidad y 3D en tiempo real
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, currentFrame * glm::radians(30.0f), glm::vec3(0.5f, 1.0f, 0.0f));

        defaultShader.setMat4("projection", projection);
        defaultShader.setMat4("view", view);
        defaultShader.setMat4("model", model);

        // Parámetros de iluminación difusa (Lambert)
        defaultShader.setVec3("lightPos", glm::vec3(2.0f, 4.0f, 3.0f));
        defaultShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        defaultShader.setVec3("objectColor", glm::vec3(0.2f, 0.65f, 0.85f));
        defaultShader.setFloat("alpha", 1.0f);

        // Dibujar la geometría de prueba
        testMesh.draw(DrawMode::Fill);

        // Renderizado de la interfaz ImGui (Panel de FPS y Rendimiento)
        editorUI.beginFrame();
        editorUI.render();
        editorUI.endFrame();

        // Intercambio de buffers y sondeo de eventos de ventana
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 8. Liberación ordenada de recursos
    editorUI.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Movimiento con teclas WASD escalado por deltaTime
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::DOWN, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Invertido: eje Y de pantalla crece hacia abajo
    lastX = xpos;
    lastY = ypos;

    // Solo rotar la cámara si se mantiene presionado el botón derecho del ratón
    // para permitir interactuar libremente con la interfaz gráfica Dear ImGui
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        camera.processMouseMovement(xoffset, yoffset);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // Ocultar y capturar el cursor para navegación libre fluida
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        } else if (action == GLFW_RELEASE) {
            // Liberar el cursor para usar la interfaz Dear ImGui
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    camera.processMouseScroll(static_cast<float>(yoffset));
}
