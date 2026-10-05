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

static void setupCustomTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Redondeos modernos
    style.WindowRounding    = 0.0f;  // Sin bordes redondeados en la ventana lateral
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 5.0f;  // Sliders, inputs y botones
    style.PopupRounding     = 6.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding      = 5.0f;  // Pestañas de sliders
    style.TabRounding       = 6.0f;  // Pestañas superiores

    // Espaciado y respiro visual
    style.WindowPadding     = ImVec2(14.0f, 14.0f);
    style.FramePadding      = ImVec2(8.0f, 6.0f);
    style.ItemSpacing       = ImVec2(10.0f, 8.0f);
    style.ItemInnerSpacing  = ImVec2(8.0f, 6.0f);
    style.ScrollbarSize     = 12.0f;

    // Paleta de colores Dark Modern (Gris Carbón + Acento Azul Eléctrico)
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]             = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg]              = ImVec4(0.09f, 0.10f, 0.12f, 0.60f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.14f, 0.15f, 0.18f, 0.98f);
    colors[ImGuiCol_Border]               = ImVec4(0.20f, 0.22f, 0.26f, 0.60f);
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.18f, 0.19f, 0.23f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.24f, 0.26f, 0.31f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.28f, 0.31f, 0.37f, 1.00f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.10f, 0.12f, 0.40f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.25f, 0.27f, 0.33f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.32f, 0.35f, 0.42f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.38f, 0.42f, 0.50f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.35f, 0.68f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.30f, 0.60f, 0.95f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.40f, 0.72f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.28f, 0.34f, 0.42f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.34f, 0.40f, 0.50f, 1.00f);
    colors[ImGuiCol_Header]               = ImVec4(0.20f, 0.23f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.26f, 0.31f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.24f, 0.52f, 0.95f, 0.60f);
    colors[ImGuiCol_Separator]            = ImVec4(0.20f, 0.22f, 0.26f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.30f, 0.55f, 0.90f, 1.00f);
    colors[ImGuiCol_SeparatorActive]      = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.26f, 0.30f, 0.36f, 1.00f);
    colors[ImGuiCol_TabActive]            = ImVec4(0.22f, 0.48f, 0.88f, 1.00f);
    colors[ImGuiCol_TabUnfocused]         = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_Text]                 = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.53f, 0.58f, 1.00f);
}

void EditorUI::init(GLFWwindow* window) {
    if (m_initialized) return;

    // 1. Crear el contexto de ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 2. Estilo visual personalizado (Modern Dark Theme)
    ImGui::StyleColorsDark();
    setupCustomTheme();

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
    ImGuiIO& io = ImGui::GetIO();
    const float sidebarWidth = 360.0f;

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - sidebarWidth, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(sidebarWidth, io.DisplaySize.y), ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove
                                 | ImGuiWindowFlags_NoResize
                                 | ImGuiWindowFlags_NoCollapse
                                 | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Panel del Editor", nullptr, windowFlags);
    renderPerformancePanel();
    ImGui::Separator();
    renderRenderSettingsPanel();
    ImGui::End();
}

void EditorUI::render(Scene& scene) {
    ImGuiIO& io = ImGui::GetIO();
    const float sidebarWidth = 360.0f;

    // Anclar la ventana al lateral derecho ocupando toda la altura de la pantalla
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - sidebarWidth, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(sidebarWidth, io.DisplaySize.y), ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove
                                 | ImGuiWindowFlags_NoResize
                                 | ImGuiWindowFlags_NoCollapse
                                 | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Panel del Editor", nullptr, windowFlags);

    if (ImGui::BeginTabBar("EditorTabBar")) {
        // Pestaña 1: Jerarquía e Inspector de Entidades
        if (ImGui::BeginTabItem("Escena")) {
            renderSceneHierarchyPanel(scene);
            ImGui::Separator();
            renderPropertiesPanel(scene);
            ImGui::EndTabItem();
        }

        // Pestaña 2: Ajustes de Render, Entorno y Rendimiento
        if (ImGui::BeginTabItem("Ajustes")) {
            renderPerformancePanel();
            ImGui::Separator();
            renderRenderSettingsPanel();
            ImGui::Separator();
            renderEnvironmentPanel(scene);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

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
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.16f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.72f, 0.20f, 0.22f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.12f, 0.14f, 1.0f));
    if (ImGui::Button("Limpiar Escena Completa", ImVec2(-1, 0))) {
        scene.clear();
        m_selectedObjectId = 0;
    }
    ImGui::PopStyleColor(3);
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
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.16f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.72f, 0.20f, 0.22f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.12f, 0.14f, 1.0f));
    if (ImGui::Button("Eliminar Entidad", ImVec2(-1, 0))) {
        scene.removeObject(m_selectedObjectId);
        m_selectedObjectId = 0;
    }
    ImGui::PopStyleColor(3);
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
