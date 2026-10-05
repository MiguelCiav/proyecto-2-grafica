#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef>

/**
 * @brief Representa los datos de un único vértice en memoria.
 * Estructura alineada e intercalada (Interleaved Vertex Buffer).
 */
struct Vertex {
    glm::vec3 Position;   // location = 0 en el Vertex Shader
    glm::vec3 Normal;     // location = 1 en el Vertex Shader
    glm::vec2 TexCoords;  // location = 2 en el Vertex Shader
};

/**
 * @brief Modos de rasterizado del polígono soportados por la malla.
 */
enum class DrawMode {
    Fill,       // GL_FILL: Caras sólidas
    Wireframe,  // GL_LINE: Alambre / líneas
    Points      // GL_POINT: Nube de vértices
};

/**
 * @brief Encapsula un sub-mallado de OpenGL con sus identificadores VAO, VBO y EBO.
 * Gestiona el ciclo de vida de los buffers en la GPU mediante RAII.
 */
class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Constructor que recibe vértices y opcionalmente índices (soporta mallas no indexadas)
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {});
    ~Mesh();

    // Regla de los Cinco (Rule of 5) para RAII en OpenGL:
    // Prohibimos copias para evitar doble liberación de buffers en GPU
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Permitimos movimiento (Move Semantics) para transferir propiedad de los buffers
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // Renderizado según el modo de dibujo seleccionado
    void draw(DrawMode mode = DrawMode::Fill) const;

    // Renderizado de un triángulo individual específico (REQ-A7)
    void drawTriangle(unsigned int triangleIndex, DrawMode mode = DrawMode::Fill) const;

    // Renderizado de líneas de normales vectoriales (REQ-A8)
    void drawNormals() const;

    // Getters de consulta
    unsigned int getVAO() const { return m_VAO; }
    unsigned int getVBO() const { return m_VBO; }
    unsigned int getEBO() const { return m_EBO; }
    bool isIndexed() const { return !indices.empty(); }
    size_t getIndexCount() const { return indices.size(); }
    size_t getVertexCount() const { return vertices.size(); }
    size_t getTriangleCount() const { return isIndexed() ? (indices.size() / 3) : (vertices.size() / 3); }

private:
    unsigned int m_VAO{0};
    unsigned int m_VBO{0};
    unsigned int m_EBO{0};

    // Buffer de líneas para inspección de normales vectoriales
    unsigned int m_normalsVAO{0};
    unsigned int m_normalsVBO{0};
    size_t m_normalsCount{0};

    void setupMesh();
    void cleanup();
};
