#pragma once

// Forward declaration de GLFWwindow para evitar incluir GLFW en cabeceras
// y prevenir conflictos con el orden estricto de inclusión de GLAD
struct GLFWwindow;

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
    void endFrame();

private:
    bool m_initialized{false};
};
