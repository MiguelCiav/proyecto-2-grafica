#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/Shader.h"
#include "core/Camera.h"
#include "graphics/Mesh.h"
#include "graphics/Model.h"
#include "scene/Scene.h"
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 4. Inicialización de Dear ImGui (Dev B)
    EditorUI editorUI;
    editorUI.init(window);

    // 5. Carga y compilación de shaders base provistos por la cátedra (Dev A)
    Shader baseShader("assets/shaders/base.vert", "assets/shaders/base.frag");

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

    Scene scene;
    auto cubeModel = std::make_shared<Model>("assets/models/cube.obj");
    auto bunnyModel = std::make_shared<Model>("assets/models/bunny.obj");
    auto teapotModel = std::make_shared<Model>("assets/models/teapot.obj");
    auto pandaModel = std::make_shared<Model>("assets/models/panda.obj");

    // 1. Suelo plano de referencia
    auto floorObj = scene.addObject("Suelo Gris", cubeModel);
    floorObj->transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
    floorObj->transform.scale = glm::vec3(7.0f, 0.1f, 7.0f);
    floorObj->color = glm::vec4(0.35f, 0.35f, 0.4f, 1.0f);

    // 2. Oso Panda 3D en el centro
    auto pandaObj = scene.addObject("Oso Panda", pandaModel);
    pandaObj->transform.position = glm::vec3(0.0f, 0.0f, 0.3f);
    pandaObj->transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    pandaObj->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    // 3. Stanford Bunny (blanco marfil) a la izquierda
    auto bunnyObj = scene.addObject("Stanford Bunny", bunnyModel);
    bunnyObj->transform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
    bunnyObj->transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    bunnyObj->color = glm::vec4(0.9f, 0.88f, 0.85f, 1.0f);

    // 4. Utah Teapot (bronce / cobre) a la derecha
    auto teapotObj = scene.addObject("Utah Teapot", teapotModel);
    teapotObj->transform.position = glm::vec3(2.0f, -0.2f, 0.0f);
    teapotObj->transform.scale = glm::vec3(0.9f, 0.9f, 0.9f);
    teapotObj->color = glm::vec4(0.85f, 0.45f, 0.2f, 1.0f);

    // 5. Cubo de referencia en modo Wireframe al fondo
    auto wireCube = scene.addObject("Cubo Alambre (Fondo)", cubeModel);
    wireCube->transform.position = glm::vec3(0.0f, 0.8f, -3.0f);
    wireCube->transform.scale = glm::vec3(1.5f, 1.5f, 1.5f);
    wireCube->color = glm::vec4(0.2f, 0.85f, 0.95f, 1.0f);
    wireCube->showWireframe = true;

    // 7. Bucle Principal de Renderizado
    while (!glfwWindowShouldClose(window)) {
        // Cálculo del tiempo por fotograma (deltaTime)
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Procesar entradas de teclado
        processInput(window);

        // Limpieza de los buffers usando el color de fondo dinámico de la escena
        const auto& bg = scene.getBackgroundColor();
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (SCR_HEIGHT > 0) ? (static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT)) : 1.0f;
        glm::mat4 projection = camera.getProjectionMatrix(aspect);
        glm::mat4 view = camera.getViewMatrix();

        // Renderizado centralizado de la escena 3D
        scene.render(baseShader, view, projection);

        // Renderizado de la interfaz ImGui completa (conectada a la escena)
        editorUI.beginFrame();
        editorUI.render(scene);
        editorUI.endFrame();

        // Intercambio de buffers y sondeo de eventos de ventana
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 8. Liberación ordenada de recursos
    scene.clear();
    cubeModel.reset();
    bunnyModel.reset();
    teapotModel.reset();
    pandaModel.reset();
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
