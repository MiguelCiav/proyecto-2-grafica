#include "EditorUI.h"
#include "scene/Scene.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <string>

EditorUI::~EditorUI() {
    shutdown();
}

void EditorUI::init(GLFWwindow* window) {
    if (m_initialized) return;

    // 1. Crear el contexto de ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 2. Estilo visual oscuro
    ImGui::StyleColorsDark();

    // 3. Inicializar backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 4. Aplicar estados iniciales de OpenGL
    setDepthTest(m_depthTest);
    setCullFace(m_cullFace);

    m_initialized = true;
}

void EditorUI::setDepthTest(bool enable) {
    m_depthTest = enable;
    if (m_depthTest) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void EditorUI::setCullFace(bool enable) {
    m_cullFace = enable;
    if (m_cullFace) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void EditorUI::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void EditorUI::render() {
    renderPerformancePanel();
    renderRenderSettingsPanel();
}

void EditorUI::render(Scene& scene) {
    // Ventana 1: Panel de Control del Motor
    ImGui::Begin("Panel de Control");
    renderPerformancePanel();
    ImGui::Separator();
    renderRenderSettingsPanel();
    ImGui::Separator();
    renderEnvironmentPanel(scene);
    ImGui::End();

    // Ventana 2: Inspector de Escena y Entidades
    ImGui::Begin("Inspector de Escena");
    renderSceneHierarchyPanel(scene);
    ImGui::Separator();
    renderPropertiesPanel(scene);
    ImGui::End();
}

void EditorUI::renderPerformancePanel() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Rendimiento:");
    ImGui::Text("  FPS: %.1f", io.Framerate);
    ImGui::Text("  Frametime: %.3f ms", 1000.0f / (io.Framerate > 0.0f ? io.Framerate : 1.0f));
}

void EditorUI::renderRenderSettingsPanel() {
    ImGui::Text("Configuracion de Render:");
    if (ImGui::Checkbox("Depth Test (GL_DEPTH_TEST)", &m_depthTest)) {
        setDepthTest(m_depthTest);
    }
    if (ImGui::Checkbox("Back-Face Culling (GL_CULL_FACE)", &m_cullFace)) {
        setCullFace(m_cullFace);
    }
}

void EditorUI::renderEnvironmentPanel(Scene& scene) {
    ImGui::Text("Entorno e Iluminacion:");
    
    // Color de fondo de la ventana
    ImGui::ColorEdit4("Color de Fondo", glm::value_ptr(scene.getBackgroundColor()));

    // Parámetros de la luz direccional global
    auto& light = scene.getLight();
    ImGui::DragFloat3("Dir. Luz", glm::value_ptr(light.direction), 0.02f, -1.0f, 1.0f);
    ImGui::ColorEdit3("Color Luz", glm::value_ptr(light.color));
    ImGui::ColorEdit3("Luz Ambiental", glm::value_ptr(light.ambient));

    ImGui::Spacing();
    if (ImGui::Button("Limpiar Escena Completa", ImVec2(-1, 0))) {
        scene.clear();
        m_selectedObjectId = 0;
    }
}

void EditorUI::renderSceneHierarchyPanel(Scene& scene) {
    const auto& objects = scene.getObjects();
    ImGui::Text("Jerarquia de Entidades (%zu):", objects.size());

    ImGui::BeginChild("ListaEntidades", ImVec2(0, 150), true);
    for (const auto& obj : objects) {
        if (!obj) continue;

        bool isSelected = (obj->id == m_selectedObjectId);
        std::string label = obj->name + " (ID: " + std::to_string(obj->id) + ")";

        if (ImGui::Selectable(label.c_str(), isSelected)) {
            m_selectedObjectId = obj->id;
        }
    }
    ImGui::EndChild();
}

void EditorUI::renderPropertiesPanel(Scene& scene) {
    ImGui::Text("Propiedades de la Entidad:");

    auto obj = scene.getObject(m_selectedObjectId);
    if (!obj) {
        ImGui::TextDisabled("Selecciona una entidad de la jerarquia.");
        return;
    }

    // Edición de nombre
    char nameBuffer[128];
    std::strncpy(nameBuffer, obj->name.c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    if (ImGui::InputText("Nombre", nameBuffer, sizeof(nameBuffer))) {
        obj->name = nameBuffer;
    }

    // Sliders de Transformación
    ImGui::Spacing();
    ImGui::Text("Transformacion:");
    ImGui::DragFloat3("Posicion", glm::value_ptr(obj->transform.position), 0.05f);
    ImGui::DragFloat3("Rotacion", glm::value_ptr(obj->transform.rotation), 1.0f, -360.0f, 360.0f);
    ImGui::DragFloat3("Escala", glm::value_ptr(obj->transform.scale), 0.05f, 0.01f, 50.0f);

    // Material y Color Difuso
    ImGui::Spacing();
    ImGui::Text("Material:");
    ImGui::ColorEdit4("Color Difuso", glm::value_ptr(obj->color));

    // Modos de Visualización
    ImGui::Spacing();
    ImGui::Text("Modos de Visualizacion:");
    ImGui::Checkbox("Visible", &obj->visible);
    ImGui::Checkbox("Alambre (Wireframe)", &obj->showWireframe);
    ImGui::Checkbox("Vertices (Puntos)", &obj->showVertices);
    ImGui::Checkbox("Mostrar Normales", &obj->showNormals);
    ImGui::Checkbox("Bounding Box", &obj->showBoundingBox);

    // Botón para eliminar entidad individual
    ImGui::Spacing();
    if (ImGui::Button("Eliminar Entidad", ImVec2(-1, 0))) {
        scene.removeObject(m_selectedObjectId);
        m_selectedObjectId = 0;
    }
}

void EditorUI::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorUI::shutdown() {
    if (m_initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        m_initialized = false;
    }
}
