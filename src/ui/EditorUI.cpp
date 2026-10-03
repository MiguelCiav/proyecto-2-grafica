#include "EditorUI.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

EditorUI:: ~EditorUI(){
    shutdown();
}

void EditorUI::init(GLFWwindow* window){
    if (m_initialized) return;

    // 1. Crear el contexto de ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 2. Establecer el estilo visual oscuro (moderno)
    ImGui::StyleColorsDark();

    // 3. Inicializar el backend de GLFW (el segundo parámetro true instala
    //    automáticamente los callbacks de teclado y ratón de ImGui)
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    // 4. Inicializar el backend de OpenGL 3 indicando la versión GLSL 330 Core
    ImGui_ImplOpenGL3_Init("#version 330");
    m_initialized = true;
}

void EditorUI::beginFrame(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


void EditorUI::render() {
    ImGuiIO& io = ImGui::GetIO();

    // Ventana flotante de Rendimiento
    ImGui::Begin("Rendimiento");
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Frametime: %.3f ms", 1000.0f / (io.Framerate > 0.0f ? io.Framerate : 1.0f));
    ImGui::End();
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
