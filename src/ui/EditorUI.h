#pragma once

#include "scene/Scene.h"

// Forward declaration de GLFWwindow para evitar incluir GLFW en cabeceras
// y prevenir conflictos con el orden estricto de inclusión de GLAD
struct GLFWwindow;

/**
 * @brief Gestiona la interfaz gráfica de usuario con Dear ImGui.
 * Proporciona paneles para inspección de escena, transformaciones,
 * estados de renderizado (Depth Test, Culling) y entorno de iluminación.
 */
class EditorUI {
public:
    EditorUI() = default;
    ~EditorUI();

    // Inicialización y limpieza del contexto de ImGui
    void init(GLFWwindow* window);
    void shutdown();

    // Ciclo por fotograma
    void beginFrame();
    void render();
    void render(Scene& scene);
    void endFrame();

    // Estados de rasterizado de OpenGL
    bool isDepthTestEnabled() const { return m_depthTest; }
    void setDepthTest(bool enable);
    bool isCullFaceEnabled() const { return m_cullFace; }
    void setCullFace(bool enable);

    // Selección de entidades y modos (Global, Local, Triángulo)
    SelectionMode getSelectionMode() const { return m_selectionMode; }
    void setSelectionMode(SelectionMode mode) { m_selectionMode = mode; }

    unsigned int getSelectedObjectId() const { return m_selectedObjectId; }
    void setSelectedObjectId(unsigned int id) { m_selectedObjectId = id; }

    int getSelectedSubMeshIndex() const { return m_selectedSubMeshIndex; }
    void setSelectedSubMeshIndex(int index) { m_selectedSubMeshIndex = index; }

private:

    bool m_initialized{false};

    // Estados configurables de renderizado
    bool m_depthTest{true};
    bool m_cullFace{false};

    // Entidad y sub-mallado actualmente seleccionados en el inspector
    SelectionMode m_selectionMode{SelectionMode::Global};
    unsigned int m_selectedObjectId{0};
    int m_selectedSubMeshIndex{-1};

    // Paneles modulares de interfaz
    void renderPerformancePanel();
    void renderRenderSettingsPanel();
    void renderEnvironmentPanel(Scene& scene);
    void renderPersistencePanel(Scene& scene);
    void renderSceneHierarchyPanel(Scene& scene);
    void renderPrimitivesCreatorPanel(Scene& scene);
    void renderPropertiesPanel(Scene& scene);

    // Persistencia de escena (.scene)
    char m_sceneFilePathBuffer[256]{"assets/scenes/default.scene"};
    std::string m_persistenceStatus;
    bool m_persistenceStatusIsError{false};
};