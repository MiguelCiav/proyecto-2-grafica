#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/Shader.h"
#include "core/Camera.h"
#include "graphics/Mesh.h"
#include "graphics/Model.h"
#include "scene/Scene.h"
#include "scene/SceneSerializer.h"
#include "ui/EditorUI.h"
#include "graphics/Framebuffer.h"
#include <imgui.h>

#include <iostream>
#include <vector>

// Configuración inicial de la ventana
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;

// Puntero global para redimensionamiento del FBO de picking y estado de selección
Framebuffer* g_pickingFBO = nullptr;
bool pendingPick = false;
int pickX = 0;
int pickY = 0;

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

int main(int argc, char* argv[]) {
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
    Shader pickingShader("assets/shaders/picking.vert", "assets/shaders/picking.frag");
    Shader debugShader("assets/shaders/debug.vert", "assets/shaders/debug.frag");

    // Framebuffer fuera de pantalla para Color Picking (Dev A)
    Framebuffer pickingFBO(SCR_WIDTH, SCR_HEIGHT);
    g_pickingFBO = &pickingFBO;

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
    auto cubeObj = scene.addObject("Cubo", cubeModel);
    cubeObj->color = glm::vec4(0.2f, 0.7f, 0.95f, 1.0f);

    // Modo de prueba automatizada para SceneSerializer
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--test-serializer") {
            SceneSerializer serializer(scene);
            std::cout << "[TEST] Probando SceneSerializer::serialize..." << std::endl;
            if (!serializer.serialize("assets/scenes/test_run.scene")) {
                std::cerr << "[TEST FAIL] Error al serializar: " << serializer.getLastError() << std::endl;
                return 1;
            }
            std::cout << "[TEST] Probando Scene::clear()..." << std::endl;
            scene.clear();
            if (!scene.getObjects().empty()) {
                std::cerr << "[TEST FAIL] Scene::clear() no vacio la lista de objetos." << std::endl;
                return 1;
            }
            std::cout << "[TEST] Probando SceneSerializer::deserialize..." << std::endl;
            if (!serializer.deserialize("assets/scenes/test_run.scene")) {
                std::cerr << "[TEST FAIL] Error al deserializar: " << serializer.getLastError() << std::endl;
                return 1;
            }
            if (scene.getObjects().size() != 1) {
                std::cerr << "[TEST FAIL] Se esperaba 1 objeto y hay " << scene.getObjects().size() << std::endl;
                return 1;
            }
            std::cout << "[TEST] Probando deserialize en assets/scenes/demo.scene..." << std::endl;
            if (!serializer.deserialize("assets/scenes/demo.scene")) {
                std::cerr << "[TEST FAIL] Error al deserializar demo.scene: " << serializer.getLastError() << std::endl;
                return 1;
            }
            if (scene.getObjects().size() != 4) {
                std::cerr << "[TEST FAIL] Se esperaban 4 objetos en demo.scene y hay " << scene.getObjects().size() << std::endl;
                return 1;
            }
            std::cout << "[TEST SUCCESS] Todas las pruebas de SceneSerializer pasaron correctamente!" << std::endl;
            glfwDestroyWindow(window);
            glfwTerminate();
            return 0;
        }
    }

    // 7. Bucle Principal de Renderizado
    while (!glfwWindowShouldClose(window)) {
        // Cálculo del tiempo por fotograma (deltaTime)
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Procesar entradas de teclado
        processInput(window);

        // Limpieza de buffers usando el color de fondo dinámico de la escena
        const auto& bg = scene.getBackgroundColor();
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (SCR_HEIGHT > 0) ? (static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT)) : 1.0f;
        glm::mat4 projection = camera.getProjectionMatrix(aspect);
        glm::mat4 view = camera.getViewMatrix();

        // 7.1 Pasada optimizada de Color Picking (Dev A: solo se ejecuta ante clics en el viewport 3D)
        if (pendingPick) {
            pendingPick = false;

            pickingFBO.bind();
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            scene.renderForPicking(pickingShader, view, projection, editorUI.getSelectionMode());

            unsigned int pickedID = pickingFBO.readPixelID(pickX, pickY);
            pickingFBO.unbind();

            // Restaurar estados normales de rasterizado
            glEnable(GL_BLEND);
            if (!editorUI.isDepthTestEnabled()) {
                glDisable(GL_DEPTH_TEST);
            }

            // Aplicar selección en EditorUI según el modo activo
            if (editorUI.getSelectionMode() == SelectionMode::Global) {
                editorUI.setSelectedObjectId(pickedID);
                editorUI.setSelectedSubMeshIndex(-1);
                editorUI.setSelectedTriangleIndex(-1);
            } else if (editorUI.getSelectionMode() == SelectionMode::Local) {
                if (pickedID == 0) {
                    editorUI.setSelectedObjectId(0);
                    editorUI.setSelectedSubMeshIndex(-1);
                    editorUI.setSelectedTriangleIndex(-1);
                } else {
                    unsigned int objId = 0;
                    int subIdx = -1;
                    Framebuffer::decodeLocalID(pickedID, objId, subIdx);
                    editorUI.setSelectedObjectId(objId);
                    editorUI.setSelectedSubMeshIndex(subIdx);
                    editorUI.setSelectedTriangleIndex(-1);
                }
            } else if (editorUI.getSelectionMode() == SelectionMode::Triangle) {
                if (pickedID == 0) {
                    editorUI.setSelectedObjectId(0);
                    editorUI.setSelectedSubMeshIndex(-1);
                    editorUI.setSelectedTriangleIndex(-1);
                } else {
                    TriangleHit hit;
                    if (scene.getTriangleHit(pickedID, hit)) {
                        editorUI.setSelectedObjectId(hit.objectId);
                        editorUI.setSelectedSubMeshIndex(hit.subMeshIndex);
                        editorUI.setSelectedTriangleIndex(hit.localTriangleIndex);
                    } else {
                        editorUI.setSelectedObjectId(0);
                        editorUI.setSelectedSubMeshIndex(-1);
                        editorUI.setSelectedTriangleIndex(-1);
                    }
                }
            }
        }

        // Renderizado centralizado de la escena 3D
        scene.render(baseShader, view, projection);

        // 7.2 Marcado visual del triángulo seleccionado (REQ-A7 - Requisito Parejas)
        if (editorUI.getSelectionMode() == SelectionMode::Triangle && editorUI.getSelectedTriangleIndex() >= 0) {
            auto selObj = scene.getObject(editorUI.getSelectedObjectId());
            if (selObj && selObj->visible && selObj->model) {
                int sIdx = editorUI.getSelectedSubMeshIndex();
                if (sIdx < 0 && !selObj->model->getSubMeshes().empty()) {
                    sIdx = 0;
                }
                const auto& subMeshes = selObj->model->getSubMeshes();
                if (sIdx >= 0 && sIdx < static_cast<int>(subMeshes.size())) {
                    const auto& sm = subMeshes[sIdx];
                    unsigned int triIdx = static_cast<unsigned int>(editorUI.getSelectedTriangleIndex());

                    baseShader.use();
                    baseShader.setMat4("model", selObj->getModelMatrix());

                    // 1. Dibujar cara rellena con color ámbar brillante (sin Z-fighting mediante polygon offset)
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(-2.0f, -2.0f);
                    baseShader.setVec4("objectColor", glm::vec4(1.0f, 0.85f, 0.1f, 0.95f));
                    sm.mesh.drawTriangle(triIdx, DrawMode::Fill);
                    glDisable(GL_POLYGON_OFFSET_FILL);

                    // 2. Delinear aristas con wireframe destacado en color rojo vivo
                    glEnable(GL_POLYGON_OFFSET_LINE);
                    glPolygonOffset(-3.0f, -3.0f);
                    glLineWidth(3.0f);
                    baseShader.setVec4("objectColor", glm::vec4(1.0f, 0.2f, 0.2f, 1.0f));
                    sm.mesh.drawTriangle(triIdx, DrawMode::Wireframe);
                    glLineWidth(1.0f);
                    glDisable(GL_POLYGON_OFFSET_LINE);
                }
            }
        }

        // 7.3 Herramientas de Inspección Geométrica Avanzada (REQ-A8: Normales, Vértices, Bounding Box)
        scene.renderDebug(debugShader, view, projection, editorUI.getPointSize());

        // Renderizado de la interfaz gráfica completa con pestañas e inspector
        editorUI.beginFrame();
        editorUI.render(scene);
        editorUI.endFrame();

        // Intercambio de buffers y sondeo de eventos de ventana
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 8. Liberación ordenada de recursos
    g_pickingFBO = nullptr;
    scene.clear();
    cubeModel.reset();
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
    if (g_pickingFBO) {
        g_pickingFBO->rescale(width, height);
    }
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
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Optimización: Solo registrar picking si el clic se realiza sobre el viewport 3D (no sobre ImGui)
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            pendingPick = true;
            pickX = static_cast<int>(xpos);
            pickY = static_cast<int>(ypos);
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    camera.processMouseScroll(static_cast<float>(yoffset));
}
