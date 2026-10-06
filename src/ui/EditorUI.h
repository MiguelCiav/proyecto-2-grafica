#pragma once

#include "scene/Scene.h"

struct GLFWwindow;

/**
 * @brief Gestiona la interfaz gráfica de usuario con Dear ImGui.
 * Proporciona paneles para inspección de escena, transformaciones,
 * estados de renderizado (Depth Test, Culling) y entorno de iluminación.
 */
class EditorUI
{
public:
    EditorUI() = default;
    ~EditorUI();

    void init(GLFWwindow *window);
    void shutdown();

    void beginFrame();
    void render();
    void render(Scene &scene);
    void endFrame();

    bool isDepthTestEnabled() const { return m_depthTest; }
    void setDepthTest(bool enable);
    bool isCullFaceEnabled() const { return m_cullFace; }
    void setCullFace(bool enable);

    SelectionMode getSelectionMode() const { return m_selectionMode; }
    void setSelectionMode(SelectionMode mode) { m_selectionMode = mode; }

    unsigned int getSelectedObjectId() const { return m_selectedObjectId; }
    void setSelectedObjectId(unsigned int id) { m_selectedObjectId = id; }

    int getSelectedSubMeshIndex() const { return m_selectedSubMeshIndex; }
    void setSelectedSubMeshIndex(int index) { m_selectedSubMeshIndex = index; }

    int getSelectedTriangleIndex() const { return m_selectedTriangleIndex; }
    void setSelectedTriangleIndex(int index) { m_selectedTriangleIndex = index; }

    float getPointSize() const { return m_pointSize; }
    void setPointSize(float size) { m_pointSize = size; }

private:
    bool m_initialized{false};

    bool m_depthTest{true};
    bool m_cullFace{false};

    float m_pointSize{6.0f};

    SelectionMode m_selectionMode{SelectionMode::Global};
    unsigned int m_selectedObjectId{0};
    int m_selectedSubMeshIndex{-1};
    int m_selectedTriangleIndex{-1};

    void renderTopBar();
    void renderPerformancePanel();
    void renderRenderSettingsPanel();
    void renderEnvironmentPanel(Scene &scene);
    void renderPersistencePanel(Scene &scene);
    void renderLoadSceneModal(Scene &scene);
    void renderSaveSceneModal(Scene &scene);
    void renderLoadModelModal(Scene &scene);
    void renderSceneHierarchyPanel(Scene &scene);
    void renderPrimitivesCreatorPanel(Scene &scene);
    void renderModelImporterPanel(Scene &scene);
    void renderPropertiesPanel(Scene &scene);

    char m_sceneFilePathBuffer[256]{"assets/scenes/default.scene"};
    char m_newSceneFileNameBuffer[128]{"mi_escena"};
    std::string m_persistenceStatus;
    bool m_persistenceStatusIsError{false};
    std::vector<std::string> m_availableSceneFiles;
    int m_selectedSceneIndex{0};

    bool m_showLoadModal{false};
    bool m_showSaveModal{false};

    void refreshAvailableSceneFiles();

    char m_modelFilePathBuffer[256]{"assets/models/robot.obj"};
    std::string m_modelImportStatus;
    bool m_modelImportStatusIsError{false};
    std::vector<std::string> m_availableModelFiles;
    int m_selectedModelIndex{0};
    bool m_showLoadModelModal{false};

    void refreshAvailableModelFiles();
};