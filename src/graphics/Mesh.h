#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstddef>

/**
 * @brief Representa los datos de un único vértice en memoria.
 * Estructura alineada e intercalada (Interleaved Vertex Buffer).
 */
struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

/**
 * @brief Modos de rasterizado del polígono soportados por la malla.
 */
enum class DrawMode
{
    Fill,
    Wireframe,
    Points
};

/**
 * @brief Encapsula un sub-mallado de OpenGL con sus identificadores VAO, VBO y EBO.
 * Gestiona el ciclo de vida de los buffers en la GPU mediante RAII.
 */
class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices = {});
    ~Mesh();

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    Mesh(Mesh &&other) noexcept;
    Mesh &operator=(Mesh &&other) noexcept;

    void draw(DrawMode mode = DrawMode::Fill) const;

    void drawTriangle(unsigned int triangleIndex, DrawMode mode = DrawMode::Fill) const;

    void drawNormals() const;

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

    unsigned int m_normalsVAO{0};
    unsigned int m_normalsVBO{0};
    size_t m_normalsCount{0};

    void setupMesh();
    void cleanup();
};
