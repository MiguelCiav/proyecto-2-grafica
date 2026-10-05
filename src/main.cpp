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
#include "graphics/Primitives.h"
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
void processInput(GLFWwindow* window, Scene& scene, EditorUI& editorUI);

GLFWwindow* initWindow(unsigned int width, unsigned int height, const char* title) {
    // 1. Inicialización y configuración de GLFW
    if (!glfwInit()) {
        std::cerr << "[ERROR::GLFW] Falló la inicialización de GLFW." << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 2. Creación de la ventana GLFW
    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "[ERROR::GLFW] Falló la creación de la ventana GLFW." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    // Registro de callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Habilitar V-Sync para estabilidad
    glfwSwapInterval(1);

    // 3. Inicialización de punteros OpenGL con GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[ERROR::GLAD] Falló la inicialización de punteros de OpenGL con GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    // Configuración inicial del viewport y estados de OpenGL
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

void setupInitialScene(Scene& scene) {
    // A. Modelo OBJ Multi-Mallado para demostración de Selección Local (6 sub-mallados)
    auto robotModel = std::make_shared<Model>("assets/models/robot.obj");
    auto robotObj = scene.addObject("Droide (Multi-Malla)", robotModel);
    robotObj->transform.position = glm::vec3(-2.2f, 0.0f, 0.0f);

    // B. Primitivas matemáticas procedimentales (Dev A)
    auto sphereModel = Primitives::createSphere(0.85f, 32, 16);
    auto sphereObj = scene.addObject("Esfera Procedimental", sphereModel);
    sphereObj->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sphereObj->color = glm::vec4(0.95f, 0.45f, 0.2f, 1.0f);

    auto cylinderModel = Primitives::createCylinder(0.6f, 1.6f, 32);
    auto cylinderObj = scene.addObject("Cilindro Procedimental", cylinderModel);
    cylinderObj->transform.position = glm::vec3(2.2f, 0.0f, 0.0f);
    cylinderObj->color = glm::vec4(0.3f, 0.85f, 0.35f, 1.0f);
}

void executePickingPass(Scene& scene, EditorUI& editorUI, Framebuffer& pickingFBO,
                        Shader& pickingShader, const glm::mat4& view, const glm::mat4& projection) {
    if (!pendingPick) return;
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

    // Aplicar selección en EditorUI y Scene según el modo activo
    if (editorUI.getSelectionMode() == SelectionMode::Global) {
        scene.selectObject(pickedID);
        editorUI.setSelectedObjectId(pickedID);
        editorUI.setSelectedSubMeshIndex(-1);
        editorUI.setSelectedTriangleIndex(-1);
    } else if (editorUI.getSelectionMode() == SelectionMode::Local) {
        if (pickedID == 0) {
            scene.selectObject(0);
            editorUI.setSelectedObjectId(0);
            editorUI.setSelectedSubMeshIndex(-1);
            editorUI.setSelectedTriangleIndex(-1);
        } else {
            unsigned int objId = 0;
            int subIdx = -1;
            Framebuffer::decodeLocalID(pickedID, objId, subIdx);
            scene.selectObject(objId);
            editorUI.setSelectedObjectId(objId);
            editorUI.setSelectedSubMeshIndex(subIdx);
            editorUI.setSelectedTriangleIndex(-1);
        }
    } else if (editorUI.getSelectionMode() == SelectionMode::Triangle) {
        if (pickedID == 0) {
            scene.selectObject(0);
            editorUI.setSelectedObjectId(0);
            editorUI.setSelectedSubMeshIndex(-1);
            editorUI.setSelectedTriangleIndex(-1);
        } else {
            TriangleHit hit;
            if (scene.getTriangleHit(pickedID, hit)) {
                scene.selectObject(hit.objectId);
                editorUI.setSelectedObjectId(hit.objectId);
                editorUI.setSelectedSubMeshIndex(hit.subMeshIndex);
                editorUI.setSelectedTriangleIndex(hit.localTriangleIndex);
            } else {
                scene.selectObject(0);
                editorUI.setSelectedObjectId(0);
                editorUI.setSelectedSubMeshIndex(-1);
                editorUI.setSelectedTriangleIndex(-1);
            }
        }
    }
}

void renderSelectionHighlights(Scene& scene, EditorUI& editorUI,
                               Shader& baseShader, Shader& debugShader,
                               const glm::mat4& view, const glm::mat4& projection) {
    // 1. Marcado visual del triángulo seleccionado en Modo Triángulo (REQ-A7)
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

                // Dibujar cara rellena con color ámbar brillante (polygon offset para evitar Z-fighting)
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(-2.0f, -2.0f);
                baseShader.setVec4("objectColor", glm::vec4(1.0f, 0.85f, 0.1f, 0.95f));
                sm.mesh.drawTriangle(triIdx, DrawMode::Fill);
                glDisable(GL_POLYGON_OFFSET_FILL);

                // Delinear aristas con wireframe rojo vivo
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

    // 2. Marcado visual del sub-mallado seleccionado en Modo Local
    if (editorUI.getSelectionMode() == SelectionMode::Local && editorUI.getSelectedSubMeshIndex() >= 0) {
        auto selObj = scene.getObject(editorUI.getSelectedObjectId());
        if (selObj && selObj->visible && selObj->model) {
            int sIdx = editorUI.getSelectedSubMeshIndex();
            const auto& subMeshes = selObj->model->getSubMeshes();
            if (sIdx >= 0 && sIdx < static_cast<int>(subMeshes.size())) {
                const auto& sm = subMeshes[sIdx];
                debugShader.use();
                debugShader.setMat4("projection", projection);
                debugShader.setMat4("view", view);
                debugShader.setMat4("model", selObj->getModelMatrix());
                debugShader.setVec4("debugColor", glm::vec4(0.18f, 0.76f, 0.98f, 1.0f)); // Contorno cian eléctrico

                glEnable(GL_POLYGON_OFFSET_LINE);
                glPolygonOffset(-2.0f, -2.0f);
                glLineWidth(2.5f);
                sm.mesh.draw(DrawMode::Wireframe);
                glLineWidth(1.0f);
                glDisable(GL_POLYGON_OFFSET_LINE);
            }
        }
    }
}

void cleanup(GLFWwindow* window, EditorUI& editorUI, Scene& scene) {
    g_pickingFBO = nullptr;
    scene.clear();
    editorUI.shutdown();
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // 1. Inicialización de ventana y contexto OpenGL
    GLFWwindow* window = initWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto #2 - Computación Gráfica (UCV)");
    if (!window) return -1;

    // 2. Inicialización de Dear ImGui
    EditorUI editorUI;
    editorUI.init(window);

    // 3. Shaders y Framebuffer de Color Picking
    Shader baseShader("assets/shaders/base.vert", "assets/shaders/base.frag");
    Shader pickingShader("assets/shaders/picking.vert", "assets/shaders/picking.frag");
    Shader debugShader("assets/shaders/debug.vert", "assets/shaders/debug.frag");

    Framebuffer pickingFBO(SCR_WIDTH, SCR_HEIGHT);
    g_pickingFBO = &pickingFBO;

    // 4. Configuración de entidades iniciales de la Escena
    Scene scene;
    setupInitialScene(scene);

    // 5. Bucle Principal de Renderizado
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, scene, editorUI);

        // Limpieza de buffers usando el color de fondo dinámico de la escena
        const auto& bg = scene.getBackgroundColor();
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (SCR_HEIGHT > 0) ? (static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT)) : 1.0f;
        glm::mat4 projection = camera.getProjectionMatrix(aspect);
        glm::mat4 view = camera.getViewMatrix();

        // Pasadas de renderizado
        executePickingPass(scene, editorUI, pickingFBO, pickingShader, view, projection);

        scene.update(deltaTime);
        scene.render(baseShader, view, projection);

        renderSelectionHighlights(scene, editorUI, baseShader, debugShader, view, projection);
        scene.renderDebug(debugShader, view, projection, editorUI.getPointSize());

        // Interfaz de usuario Dear ImGui
        editorUI.beginFrame();
        editorUI.render(scene);
        editorUI.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 6. Liberación ordenada de recursos
    cleanup(window, editorUI, scene);
    return 0;
}

void processInput(GLFWwindow* window, Scene& scene, EditorUI& editorUI) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Comprobar si ImGui está capturando el teclado (por ejemplo, escribiendo en un InputText)
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput || io.WantCaptureKeyboard) {
        return;
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

    // Atajo de teclado: Supr (Delete) o Backspace (Retroceso) para borrar la entidad seleccionada
    static bool deleteKeyWasDown = false;
    bool deleteKeyDown = (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) ||
                         (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS);

    if (deleteKeyDown && !deleteKeyWasDown) {
        unsigned int selId = editorUI.getSelectedObjectId();
        if (selId != 0) {
            scene.removeObject(selId);
            editorUI.setSelectedObjectId(0);
            editorUI.setSelectedSubMeshIndex(-1);
            editorUI.setSelectedTriangleIndex(-1);
        }
    }
    deleteKeyWasDown = deleteKeyDown;
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
