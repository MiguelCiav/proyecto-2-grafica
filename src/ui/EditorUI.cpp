#include "EditorUI.h"
#include "scene/Scene.h"
#include "scene/SceneSerializer.h"
#include "graphics/Primitives.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iostream>

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

    // 5. Escanear escenas disponibles en assets/scenes
    refreshAvailableSceneFiles();

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
            renderPrimitivesCreatorPanel(scene);
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

        // Pestaña 3: Archivo y Persistencia de Escena
        if (ImGui::BeginTabItem("Archivo")) {
            renderPersistencePanel(scene);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();

    // Diálogos emergentes modales (centrados en la ventana principal)
    renderLoadSceneModal(scene);
    renderSaveSceneModal(scene);
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

void EditorUI::refreshAvailableSceneFiles() {
    m_availableSceneFiles.clear();
    const std::string scenesDir = "assets/scenes";
    try {
        if (!std::filesystem::exists(scenesDir)) {
            std::filesystem::create_directories(scenesDir);
        }
        for (const auto& entry : std::filesystem::directory_iterator(scenesDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".scene") {
                m_availableSceneFiles.push_back(entry.path().filename().string());
            }
        }
        std::sort(m_availableSceneFiles.begin(), m_availableSceneFiles.end());
    } catch (const std::exception& e) {
        std::cerr << "[EditorUI WARN]: No se pudo listar assets/scenes: " << e.what() << std::endl;
    }

    if (m_availableSceneFiles.empty()) {
        m_availableSceneFiles.push_back("default.scene");
    }

    if (m_selectedSceneIndex < 0 || m_selectedSceneIndex >= static_cast<int>(m_availableSceneFiles.size())) {
        m_selectedSceneIndex = 0;
    }
}

void EditorUI::renderPersistencePanel(Scene& scene) {
    ImGui::Text("Persistencia de Escena 3D:");
    ImGui::TextDisabled("Formato propio estructurado (.scene)");
    ImGui::Spacing();

    size_t objCount = scene.getObjects().size();
    ImGui::Text("Resumen de Escena Activa:");
    ImGui::BulletText("Entidades en pantalla: %zu", objCount);
    const auto& bg = scene.getBackgroundColor();
    ImGui::BulletText("Color de fondo: (%.2f, %.2f, %.2f)", bg.r, bg.g, bg.b);
    const auto& light = scene.getLight();
    ImGui::BulletText("Luz Direccional: (%.2f, %.2f, %.2f)", light.direction.x, light.direction.y, light.direction.z);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Acciones:");

    // Botón para abrir la ventana emergente modal de Carga
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.38f, 0.62f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.48f, 0.78f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.30f, 0.50f, 1.0f));
    if (ImGui::Button("Cargar Escena...", ImVec2(-1, 38))) {
        refreshAvailableSceneFiles();
        m_showLoadModal = true;
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();

    // Botón para abrir la ventana emergente modal de Guardado
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.48f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.60f, 0.40f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.38f, 0.25f, 1.0f));
    if (ImGui::Button("Guardar Escena...", ImVec2(-1, 38))) {
        refreshAvailableSceneFiles();
        m_showSaveModal = true;
    }
    ImGui::PopStyleColor(3);

    // Mensaje de estado de la última operación
    if (!m_persistenceStatus.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if (m_persistenceStatusIsError) {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", m_persistenceStatus.c_str());
        } else {
            ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.45f, 1.0f), "%s", m_persistenceStatus.c_str());
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Sección colapsable para ruta manual externa
    if (ImGui::CollapsingHeader("Ruta Manual Directa")) {
        ImGui::InputText("Ruta", m_sceneFilePathBuffer, sizeof(m_sceneFilePathBuffer));
        if (ImGui::Button("Cargar desde Ruta Manual", ImVec2(-1, 0))) {
            SceneSerializer serializer(scene);
            if (serializer.deserialize(m_sceneFilePathBuffer)) {
                m_persistenceStatus = "Escena cargada desde:\n" + std::string(m_sceneFilePathBuffer);
                m_persistenceStatusIsError = false;
                m_selectedObjectId = 0;
                m_selectedSubMeshIndex = -1;
            } else {
                m_persistenceStatus = "Error:\n" + serializer.getLastError();
                m_persistenceStatusIsError = true;
            }
        }
    }
}

void EditorUI::renderLoadSceneModal(Scene& scene) {
    if (m_showLoadModal) {
        ImGui::OpenPopup("Cargar Escena 3D##ModalDialog");
        m_showLoadModal = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 380), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Cargar Escena 3D##ModalDialog", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Selecciona una escena para cargar en el viewport:");
        ImGui::Spacing();

        ImGui::TextDisabled("Directorio: assets/scenes/");
        ImGui::SameLine(ImGui::GetWindowWidth() - 95.0f);
        if (ImGui::SmallButton("Recargar")) {
            refreshAvailableSceneFiles();
        }

        ImGui::Spacing();

        // Lista interactiva desplazable con los archivos .scene
        ImGui::BeginChild("FileListRegion", ImVec2(0, 190), true);
        if (m_availableSceneFiles.empty()) {
            ImGui::TextDisabled("No se encontraron archivos .scene en assets/scenes/");
        } else {
            for (int i = 0; i < static_cast<int>(m_availableSceneFiles.size()); ++i) {
                const bool isSelected = (m_selectedSceneIndex == i);
                std::string itemLabel = "  " + m_availableSceneFiles[i];

                if (ImGui::Selectable(itemLabel.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                    m_selectedSceneIndex = i;

                    // Doble clic para cargar de inmediato
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        std::string loadPath = "assets/scenes/" + m_availableSceneFiles[i];
                        SceneSerializer serializer(scene);
                        if (serializer.deserialize(loadPath)) {
                            m_persistenceStatus = "Escena cargada exitosamente desde:\n" + loadPath;
                            m_persistenceStatusIsError = false;
                            m_selectedObjectId = 0;
                            m_selectedSubMeshIndex = -1;
                        } else {
                            m_persistenceStatus = "Error al cargar:\n" + serializer.getLastError();
                            m_persistenceStatusIsError = true;
                        }
                        ImGui::CloseCurrentPopup();
                    }
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();

        // Archivo activo seleccionado
        std::string selectedFilename = (m_selectedSceneIndex >= 0 && m_selectedSceneIndex < static_cast<int>(m_availableSceneFiles.size()))
                                       ? m_availableSceneFiles[m_selectedSceneIndex]
                                       : "";

        if (!selectedFilename.empty()) {
            ImGui::Text("Archivo seleccionado:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "%s", selectedFilename.c_str());
        } else {
            ImGui::TextDisabled("Ningun archivo seleccionado.");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Botones inferiores de acción
        float buttonWidth = 140.0f;
        float spacing = ImGui::GetWindowWidth() - (buttonWidth * 2.0f) - 30.0f;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.38f, 0.62f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.48f, 0.78f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.30f, 0.50f, 1.0f));
        bool canLoad = !selectedFilename.empty();
        if (!canLoad) ImGui::BeginDisabled();

        if (ImGui::Button("Cargar Escena", ImVec2(buttonWidth, 32))) {
            std::string loadPath = "assets/scenes/" + selectedFilename;
            SceneSerializer serializer(scene);
            if (serializer.deserialize(loadPath)) {
                m_persistenceStatus = "Escena cargada exitosamente desde:\n" + loadPath;
                m_persistenceStatusIsError = false;
                m_selectedObjectId = 0;
                m_selectedSubMeshIndex = -1;
            } else {
                m_persistenceStatus = "Error al cargar:\n" + serializer.getLastError();
                m_persistenceStatusIsError = true;
            }
            ImGui::CloseCurrentPopup();
        }

        if (!canLoad) ImGui::EndDisabled();
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0, spacing > 10.0f ? spacing : 10.0f);

        if (ImGui::Button("Cancelar", ImVec2(buttonWidth, 32))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void EditorUI::renderSaveSceneModal(Scene& scene) {
    if (m_showSaveModal) {
        ImGui::OpenPopup("Guardar Escena 3D##ModalDialog");
        m_showSaveModal = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 390), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Guardar Escena 3D##ModalDialog", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Guardar estado actual de la escena en archivo .scene:");
        ImGui::Spacing();

        size_t objCount = scene.getObjects().size();
        ImGui::Text("Entidades que se guardaran: %zu", objCount);
        if (objCount == 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "Aviso: la escena esta vacia (se guardara sin objetos).");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 1. Guardar como nuevo archivo
        ImGui::Text("Escribe el nombre del archivo:");
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 65.0f);
        ImGui::InputText("##ModalNuevoNombre", m_newSceneFileNameBuffer, sizeof(m_newSceneFileNameBuffer));
        ImGui::SameLine();
        ImGui::Text(".scene");

        ImGui::Spacing();

        // 2. O elegir un archivo existente para sobrescribir
        ImGui::TextDisabled("O selecciona uno existente para sobrescribirlo:");
        const char* comboPreview = (m_selectedSceneIndex >= 0 && m_selectedSceneIndex < static_cast<int>(m_availableSceneFiles.size()))
                                   ? m_availableSceneFiles[m_selectedSceneIndex].c_str()
                                   : "Ninguno";
        if (ImGui::BeginCombo("##ModalComboSobrescribir", comboPreview)) {
            for (int i = 0; i < static_cast<int>(m_availableSceneFiles.size()); ++i) {
                const bool isSelected = (m_selectedSceneIndex == i);
                if (ImGui::Selectable(m_availableSceneFiles[i].c_str(), isSelected)) {
                    m_selectedSceneIndex = i;
                    std::string stem = m_availableSceneFiles[i];
                    size_t extPos = stem.find(".scene");
                    if (extPos != std::string::npos) {
                        stem = stem.substr(0, extPos);
                    }
                    std::strncpy(m_newSceneFileNameBuffer, stem.c_str(), sizeof(m_newSceneFileNameBuffer) - 1);
                    m_newSceneFileNameBuffer[sizeof(m_newSceneFileNameBuffer) - 1] = '\0';
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        std::string finalFilename = m_newSceneFileNameBuffer;
        if (finalFilename.empty()) finalFilename = "nueva_escena";
        if (finalFilename.find(".scene") == std::string::npos) {
            finalFilename += ".scene";
        }
        std::string finalPath = "assets/scenes/" + finalFilename;

        ImGui::Spacing();
        ImGui::TextDisabled("Ruta de destino: %s", finalPath.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Botones inferiores
        float buttonWidth = 140.0f;
        float spacing = ImGui::GetWindowWidth() - (buttonWidth * 2.0f) - 30.0f;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.48f, 0.32f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.60f, 0.40f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.14f, 0.38f, 0.25f, 1.0f));

        if (ImGui::Button("Guardar Escena", ImVec2(buttonWidth, 32))) {
            SceneSerializer serializer(scene);
            if (serializer.serialize(finalPath)) {
                m_persistenceStatus = "Escena guardada correctamente en:\n" + finalPath;
                m_persistenceStatusIsError = false;
                refreshAvailableSceneFiles();
                for (size_t i = 0; i < m_availableSceneFiles.size(); ++i) {
                    if (m_availableSceneFiles[i] == finalFilename) {
                        m_selectedSceneIndex = static_cast<int>(i);
                        break;
                    }
                }
            } else {
                m_persistenceStatus = "Error al guardar:\n" + serializer.getLastError();
                m_persistenceStatusIsError = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine(0, spacing > 10.0f ? spacing : 10.0f);

        if (ImGui::Button("Cancelar", ImVec2(buttonWidth, 32))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void EditorUI::renderSceneHierarchyPanel(Scene& scene) {
    // Selector interactivo de Modo de Selección por Color Picking
    ImGui::Text("Modo de Seleccion:");
    int modeInt = static_cast<int>(m_selectionMode);
    if (ImGui::RadioButton("Global", modeInt == 0)) {
        m_selectionMode = SelectionMode::Global;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Local", modeInt == 1)) {
        m_selectionMode = SelectionMode::Local;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Triangulo", modeInt == 2)) {
        m_selectionMode = SelectionMode::Triangle;
    }
    ImGui::Spacing();

    const auto& objects = scene.getObjects();
    ImGui::Text("Jerarquia de Entidades (%zu):", objects.size());

    ImGui::BeginChild("ListaEntidades", ImVec2(0, 150), true);
    for (const auto& obj : objects) {
        if (!obj) continue;

        bool isSelected = (obj->id == m_selectedObjectId);
        std::string label = obj->name + " (ID: " + std::to_string(obj->id) + ")";

        if (ImGui::Selectable(label.c_str(), isSelected)) {
            m_selectedObjectId = obj->id;
            m_selectedSubMeshIndex = -1;
            m_selectedTriangleIndex = -1;
        }
    }
    ImGui::EndChild();
}

void EditorUI::renderPrimitivesCreatorPanel(Scene& scene) {
    if (ImGui::CollapsingHeader("Añadir Primitiva Geometrica")) {
        const char* primitiveTypes[] = {"Cubo", "Piramide", "Esfera", "Cilindro"};
        static int currentType = 0;
        ImGui::Combo("Figura", &currentType, primitiveTypes, IM_ARRAYSIZE(primitiveTypes));

        static float cubeSize = 1.0f;
        static float pyrBase = 1.0f, pyrHeight = 1.0f;
        static float sphereRadius = 1.0f;
        static int sphereSectors = 32, sphereStacks = 16;
        static float cylRadius = 1.0f, cylHeight = 1.0f;
        static int cylSectors = 32;

        if (currentType == 0) { // Cubo
            ImGui::DragFloat("Arista", &cubeSize, 0.05f, 0.1f, 20.0f);
        } else if (currentType == 1) { // Pirámide
            ImGui::DragFloat("Base", &pyrBase, 0.05f, 0.1f, 20.0f);
            ImGui::DragFloat("Altura", &pyrHeight, 0.05f, 0.1f, 20.0f);
        } else if (currentType == 2) { // Esfera
            ImGui::DragFloat("Radio", &sphereRadius, 0.05f, 0.1f, 20.0f);
            ImGui::SliderInt("Sectores", &sphereSectors, 3, 64);
            ImGui::SliderInt("Anillos (Stacks)", &sphereStacks, 2, 32);
        } else if (currentType == 3) { // Cilindro
            ImGui::DragFloat("Radio", &cylRadius, 0.05f, 0.1f, 20.0f);
            ImGui::DragFloat("Altura", &cylHeight, 0.05f, 0.1f, 20.0f);
            ImGui::SliderInt("Sectores", &cylSectors, 3, 64);
        }

        if (ImGui::Button("Instanciar en Escena", ImVec2(-1, 0))) {
            std::shared_ptr<Model> newModel = nullptr;
            std::string name;

            if (currentType == 0) {
                newModel = Primitives::createCube(cubeSize);
                name = "Cubo Procedimental";
            } else if (currentType == 1) {
                newModel = Primitives::createPyramid(pyrBase, pyrHeight);
                name = "Piramide Procedimental";
            } else if (currentType == 2) {
                newModel = Primitives::createSphere(sphereRadius, sphereSectors, sphereStacks);
                name = "Esfera Procedimental";
            } else if (currentType == 3) {
                newModel = Primitives::createCylinder(cylRadius, cylHeight, cylSectors);
                name = "Cilindro Procedimental";
            }

            if (newModel) {
                auto newObj = scene.addObject(name, newModel);
                m_selectedObjectId = newObj->id;
            }
        }
    }
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

    // Si estamos en Modo Local, mostramos e interactuamos con el Sub-mallado específico
    if (m_selectionMode == SelectionMode::Local) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Seleccion Local (Sub-mallado):");
        if (obj->model && m_selectedSubMeshIndex >= 0 &&
            m_selectedSubMeshIndex < static_cast<int>(obj->model->getSubMeshes().size())) {
            auto& subMesh = obj->model->getSubMeshes()[m_selectedSubMeshIndex];
            ImGui::BulletText("Nombre: %s", subMesh.name.c_str());
            ImGui::BulletText("Indice: %d / %zu", m_selectedSubMeshIndex, obj->model->getSubMeshes().size());
            ImGui::ColorEdit4("Color Kd Sub-mallado", glm::value_ptr(subMesh.diffuseColor));
        } else {
            ImGui::TextDisabled("Haz clic en un sub-mallado en la escena para seleccionarlo.");
        }
    }

    // Si estamos en Modo Triángulo, mostramos información detallada del triángulo seleccionado
    if (m_selectionMode == SelectionMode::Triangle) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.1f, 1.0f), "Seleccion por Triangulo (Marcado Activo):");
        if (obj->model && m_selectedTriangleIndex >= 0) {
            int sIdx = (m_selectedSubMeshIndex >= 0 && m_selectedSubMeshIndex < static_cast<int>(obj->model->getSubMeshes().size()))
                       ? m_selectedSubMeshIndex : 0;
            const auto& sm = obj->model->getSubMeshes()[sIdx];
            ImGui::BulletText("Triangulo Local: #%d / %zu", m_selectedTriangleIndex, sm.mesh.getTriangleCount());
            ImGui::BulletText("Sub-mallado: %s", sm.name.c_str());

            // Inspección de vértices que componen el triángulo
            if (m_selectedTriangleIndex < static_cast<int>(sm.mesh.getTriangleCount())) {
                glm::vec3 v0(0.0f), v1(0.0f), v2(0.0f);
                if (sm.mesh.isIndexed()) {
                    unsigned int i0 = sm.mesh.indices[m_selectedTriangleIndex * 3 + 0];
                    unsigned int i1 = sm.mesh.indices[m_selectedTriangleIndex * 3 + 1];
                    unsigned int i2 = sm.mesh.indices[m_selectedTriangleIndex * 3 + 2];
                    v0 = sm.mesh.vertices[i0].Position;
                    v1 = sm.mesh.vertices[i1].Position;
                    v2 = sm.mesh.vertices[i2].Position;
                    ImGui::Text("Indices: [%u, %u, %u]", i0, i1, i2);
                } else if (!sm.mesh.vertices.empty()) {
                    v0 = sm.mesh.vertices[m_selectedTriangleIndex * 3 + 0].Position;
                    v1 = sm.mesh.vertices[m_selectedTriangleIndex * 3 + 1].Position;
                    v2 = sm.mesh.vertices[m_selectedTriangleIndex * 3 + 2].Position;
                }
                ImGui::Text("V0: (%.2f, %.2f, %.2f)", v0.x, v0.y, v0.z);
                ImGui::Text("V1: (%.2f, %.2f, %.2f)", v1.x, v1.y, v1.z);
                ImGui::Text("V2: (%.2f, %.2f, %.2f)", v2.x, v2.y, v2.z);
            }
        } else {
            ImGui::TextDisabled("Haz clic en una cara o triangulo en la escena.");
        }
    }

    // Modos de Visualización
    ImGui::Spacing();
    ImGui::Text("Modos de Visualizacion:");
    ImGui::Checkbox("Visible", &obj->visible);
    ImGui::Checkbox("Alambre (Wireframe)", &obj->showWireframe);
    ImGui::Checkbox("Vertices (Puntos)", &obj->showVertices);
    if (obj->showVertices) {
        ImGui::Indent();
        ImGui::SliderFloat("Tamaño Puntos", &m_pointSize, 2.0f, 20.0f, "%.1f px");
        ImGui::Unindent();
    }
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
        m_selectedSubMeshIndex = -1;
        m_selectedTriangleIndex = -1;
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
