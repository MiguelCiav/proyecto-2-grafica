#pragma once

// Forward declaration de GLFWwindow para evitar incluir GLFW en cabeceras
// y prevenir conflictos con el orden estricto de inclusión de GLAD
struct GLFWwindow;
class Scene;

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

    // Selección de entidades (base para la integración con Picking en INT-02)
    unsigned int getSelectedObjectId() const { return m_selectedObjectId; }
    void setSelectedObjectId(unsigned int id) { m_selectedObjectId = id; }

private:

    bool m_initialized{false};

    // Estados configurables de renderizado
    bool m_depthTest{true};
    bool m_cullFace{false};

    // Entidad actualmente seleccionada en el inspector
    unsigned int m_selectedObjectId{0};

    // Paneles modulares de interfaz
    void renderPerformancePanel();
    void renderRenderSettingsPanel();
    void renderEnvironmentPanel(Scene& scene);
    void renderSceneHierarchyPanel(Scene& scene);
    void renderPrimitivesCreatorPanel(Scene& scene);
    void renderPropertiesPanel(Scene& scene);
};