// GLAD Y GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Matemáticas para gráficos
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Interfaz gráfica (Dear ImGui)
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Código fuente del Vertex Shader
const char* vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n";

// Código fuente del Fragment Shader (fijamos un color naranja/amarillo inicial)
const char* fragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 uColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(uColor, 1.0f);\n"
    "}\n";

int main() {

    glfwInit(); // Se inicia glfw

    // Acá defino las versiones
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Acá se define el perfil (ni mucha idea)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // -----------------------------
    // ||   CREACIÓN DE VENTANA   ||
    // -----------------------------

    // 1. Crea la ventana y enlace el contexto principal
    GLFWwindow* window = glfwCreateWindow(800, 600, "Proyecto 2", NULL, NULL);

    // 2. Verificamos si la ventana se creó
    if (!window) {
        glfwTerminate(); // 2.1. Si algo falló terminamos
        return -1;
    }

    // 3. Asignamos el contexto del hilo de opengl a la ventana actual
    glfwMakeContextCurrent(window);

    // 4. Cargamos los punteros de glad en el hilo que ya asignamos
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    // 5. Acá definimos dónde se va a dibujar todo
    glViewport(0, 0, 800, 600);

    // --------------------------------
    // ||   COMPILACIÓN DE SHADERS   ||
    // --------------------------------

    // 1. Compilar Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // 2. Compilar Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 3. Crear el programa y enlazar
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 4. Liberar los shaders individuales
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ------------------------------
    // ||   DIBUJO DE TRIÁNGULOS   ||
    // ------------------------------

    float vertices[] = {
    -0.5f, -0.5f, 0.0f, // Inferior izquierdo
     0.5f, -0.5f, 0.0f, // Inferior derecho
     0.0f,  0.5f, 0.0f  // Superior centro
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --------------------------------
    // ||   INICIALIZACIÓN DE IMGUI  ||
    // --------------------------------

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float triangleColor[3] = { 0.2f, 0.8f, 0.2f }; // Color inicial (verde)
    float rotationAngle = 0.0f;                    // Ángulo en grados

    // ------------------------------
    // ||   BUCLE DE RENDERIZADO   ||
    // ------------------------------
    
    while (!glfwWindowShouldClose(window)) {
        // 1. Limpieza de pantalla
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 2. Definir la interfaz
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(350, 180), ImGuiCond_FirstUseEver);
        ImGui::Begin("Controles Geométricos");
        ImGui::ColorEdit3("Color Triángulo", triangleColor);
        ImGui::SliderFloat("Rotación Z", &rotationAngle, 0.0f, 360.0f);
        ImGui::End();
        
        // 3. Actualizar valores uniformes
        glm::mat4 transform = glm::mat4(1.0f); // Matriz identidad
        transform = glm::rotate(transform, glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));

        glUseProgram(shaderProgram);
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        unsigned int colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        glUniform3f(colorLoc, triangleColor[0], triangleColor[1], triangleColor[2]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 4. Dibujar la interfaz encima

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 5. Intercambio de buffers y eventos

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 9. Limpieza de memoria

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}