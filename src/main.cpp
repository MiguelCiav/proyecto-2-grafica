// GLAD debe incluirse SIEMPRE antes de GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Dear ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// tinyobjloader
#include <tiny_obj_loader.h>

// Utilidades estándar C++17
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "Shader.h"

// Dimensiones iniciales de la ventana
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Callback para ajuste de tamaño de ventana
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

// Búsqueda robusta de rutas de recursos
std::filesystem::path resolveAssetPath(const std::string& relativePath) {
    std::filesystem::path current = std::filesystem::current_path();
    for (int i = 0; i < 5; ++i) {
        if (std::filesystem::exists(current / relativePath)) {
            return current / relativePath;
        }
        if (std::filesystem::exists(current / "assets" / relativePath)) {
            return current / "assets" / relativePath;
        }
        if (current.has_parent_path() && current != current.parent_path()) {
            current = current.parent_path();
        } else {
            break;
        }
    }
    return relativePath;
}

// Carga de modelo con tinyobjloader
bool loadModel(const std::filesystem::path& path, std::vector<float>& outData, int& outVertexCount) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path.string(), reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "[tinyobjloader] Error: " << reader.Error() << std::endl;
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "[tinyobjloader] Advertencia: " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    outData.clear();

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            // Posición (X, Y, Z)
            outData.push_back(attrib.vertices[3 * index.vertex_index + 0]);
            outData.push_back(attrib.vertices[3 * index.vertex_index + 1]);
            outData.push_back(attrib.vertices[3 * index.vertex_index + 2]);

            // Normales (NX, NY, NZ)
            if (index.normal_index >= 0) {
                outData.push_back(attrib.normals[3 * index.normal_index + 0]);
                outData.push_back(attrib.normals[3 * index.normal_index + 1]);
                outData.push_back(attrib.normals[3 * index.normal_index + 2]);
            } else {
                // Normal por defecto apuntando hacia arriba si el archivo no la tiene
                outData.push_back(0.0f);
                outData.push_back(1.0f);
                outData.push_back(0.0f);
            }
        }
    }

    outVertexCount = static_cast<int>(outData.size() / 6);
    std::cout << "[tinyobjloader] Archivo cargado correctamente: " << path.string()
              << " (" << outVertexCount << " vertices generados)\n";
    return true;
}

// Shaders integrados de respaldo (por si los archivos externos no se encuentran)
const char* fallbackVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fallbackFragmentShader = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * lightColor;
    
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

int main() {
    // 1. Inicialización de GLFW
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW\n";
        return -1;
    }

    // Configuración para OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 2. Creación de la ventana
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto Base OpenGL 3.3 Core", nullptr, nullptr);
    if (!window) {
        std::cerr << "Error al crear la ventana GLFW\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1); // Habilitar V-Sync

    // 3. Inicialización de GLAD (Carga de punteros a funciones OpenGL)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Error al inicializar GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "===========================================\n";
    std::cout << "OpenGL Vendor   : " << glGetString(GL_VENDOR) << "\n";
    std::cout << "OpenGL Renderer : " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Version  : " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version    : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    std::cout << "===========================================\n";

    // 4. Configuración inicial del estado de OpenGL
    glEnable(GL_DEPTH_TEST);

    // 5. Inicialización de Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 6. Carga y compilación de Shaders
    auto vertPath = resolveAssetPath("shaders/basic.vert");
    auto fragPath = resolveAssetPath("shaders/basic.frag");
    Shader shader;
    if (std::filesystem::exists(vertPath) && std::filesystem::exists(fragPath)) {
        std::cout << "[Shader] Cargando desde archivos: " << vertPath << " y " << fragPath << "\n";
        shader = Shader(vertPath.string(), fragPath.string());
    } else {
        std::cout << "[Shader] Archivos no encontrados en disco. Usando shaders de respaldo embebidos.\n";
        shader.compileFromSource(fallbackVertexShader, fallbackFragmentShader);
    }

    // 7. Carga de geometría con tinyobjloader
    auto modelPath = resolveAssetPath("models/cube.obj");
    std::vector<float> vertexData;
    int vertexCount = 0;
    bool modelLoaded = loadModel(modelPath, vertexData, vertexCount);

    if (!modelLoaded || vertexCount == 0) {
        std::cerr << "[tinyobjloader] No se pudo cargar el modelo, usando cubo por defecto.\n";
        // Vértices de cubo de respaldo (36 vértices: 6 floats por vértice -> pos(3) + normal(3))
        vertexData = {
            // Front
            -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
             0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
             0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
             0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
            // Back
            -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
             0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
             0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
             0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
            -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
            -0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
            // Top
            -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
            // Bottom
            -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
             0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
             0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
             0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
            -0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
            -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
            // Right
             0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
             0.5f, 0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
             0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
             0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
             0.5f,-0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
             0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
            // Left
            -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f
        };
        vertexCount = 36;
    }

    // 8. Creación de VAO y VBO
    GLuint VAO = 0;
    GLuint VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Atributo 0: Posición (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Atributo 1: Normal (vec3)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Variables de configuración interactiva (ImGui)
    glm::vec3 clearColor = glm::vec3(0.12f, 0.14f, 0.18f);
    glm::vec3 objectColor = glm::vec3(0.2f, 0.7f, 0.9f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(2.5f, 3.0f, 3.0f);

    glm::vec3 rotationAngles = glm::vec3(20.0f, 30.0f, 0.0f);
    bool autoRotate = true;
    float autoRotateSpeed = 45.0f; // grados por segundo
    bool wireframe = false;

    float lastFrameTime = static_cast<float>(glfwGetTime());

    // 9. Bucle principal de renderizado
    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Procesar eventos de entrada
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        if (autoRotate) {
            rotationAngles.y += autoRotateSpeed * deltaTime;
            if (rotationAngles.y >= 360.0f) rotationAngles.y -= 360.0f;
        }

        // Iniciar nuevo frame de ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Ventana de control Dear ImGui
        {
            ImGui::Begin("Panel de Control 3D");
            ImGui::Text("Proyecto Base OpenGL 3.3 Core");
            ImGui::Separator();

            ImGui::Text("Rendimiento: %.1f FPS (%.2f ms/frame)", io.Framerate, 1000.0f / io.Framerate);
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Transformaciones (GLM)", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Auto-rotar", &autoRotate);
                if (autoRotate) {
                    ImGui::SliderFloat("Velocidad rotacion", &autoRotateSpeed, 5.0f, 180.0f, "%.1f deg/s");
                }
                ImGui::SliderFloat3("Angulos (X, Y, Z)", glm::value_ptr(rotationAngles), 0.0f, 360.0f, "%.1f deg");
                if (ImGui::Button("Reset Angulos")) {
                    rotationAngles = glm::vec3(0.0f);
                }
            }

            if (ImGui::CollapsingHeader("Material e Iluminacion", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::ColorEdit3("Color Objeto", glm::value_ptr(objectColor));
                ImGui::ColorEdit3("Color Luz", glm::value_ptr(lightColor));
                ImGui::DragFloat3("Posicion Luz", glm::value_ptr(lightPos), 0.1f, -10.0f, 10.0f);
                ImGui::ColorEdit3("Color Fondo", glm::value_ptr(clearColor));
            }

            if (ImGui::CollapsingHeader("Render & Geometria")) {
                if (ImGui::Checkbox("Modo Wireframe", &wireframe)) {
                    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
                }
                ImGui::Text("Modelo: %s", modelLoaded ? "assets/models/cube.obj" : "Cubo Interno");
                ImGui::Text("Total Vertices: %d", vertexCount);
                ImGui::Text("Total Triangulos: %d", vertexCount / 3);
            }

            if (ImGui::CollapsingHeader("Informacion del Sistema")) {
                ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
                ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
                ImGui::Text("OpenGL: %s", glGetString(GL_VERSION));
                ImGui::Text("GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
            }

            ImGui::End();
        }

        // Limpiar buffers
        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Obtener dimensiones actuales del framebuffer
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        float aspectRatio = (display_h > 0) ? (static_cast<float>(display_w) / static_cast<float>(display_h)) : 1.0f;

        // Matrices con GLM
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 4.5f);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotationAngles.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationAngles.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotationAngles.z), glm::vec3(0.0f, 0.0f, 1.0f));

        // Activar Shader y pasar Uniforms
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);

        shader.setVec3("objectColor", objectColor);
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", cameraPos);

        // Renderizado del objeto 3D
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);

        // Renderizado de la UI de Dear ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Intercambio de buffers
        glfwSwapBuffers(window);
    }

    // 10. Limpieza de recursos
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
