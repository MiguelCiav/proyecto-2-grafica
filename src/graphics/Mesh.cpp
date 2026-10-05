#include "Mesh.h"
#include <utility>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : vertices(vertices), indices(indices) {
    setupMesh();
}

Mesh::~Mesh() {
    cleanup();
}

Mesh::Mesh(Mesh&& other) noexcept
    : vertices(std::move(other.vertices)),
      indices(std::move(other.indices)),
      m_VAO(other.m_VAO),
      m_VBO(other.m_VBO),
      m_EBO(other.m_EBO),
      m_normalsVAO(other.m_normalsVAO),
      m_normalsVBO(other.m_normalsVBO),
      m_normalsCount(other.m_normalsCount) {
    // Anulamos los identificadores en el objeto movido para evitar que su destructor los libere en GPU
    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_EBO = 0;
    other.m_normalsVAO = 0;
    other.m_normalsVBO = 0;
    other.m_normalsCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        // Liberamos los recursos actuales de este objeto en GPU
        cleanup();

        // Transferimos la propiedad de los datos y buffers
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        m_VAO = other.m_VAO;
        m_VBO = other.m_VBO;
        m_EBO = other.m_EBO;
        m_normalsVAO = other.m_normalsVAO;
        m_normalsVBO = other.m_normalsVBO;
        m_normalsCount = other.m_normalsCount;

        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
        other.m_normalsVAO = 0;
        other.m_normalsVBO = 0;
        other.m_normalsCount = 0;
    }
    return *this;
}

void Mesh::setupMesh() {
    // 1. Generar contenedores e identificadores en la GPU
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    if (isIndexed()) {
        glGenBuffers(1, &m_EBO);
    }

    // 2. Enlazar el VAO primero para registrar toda la configuración posterior
    glBindVertexArray(m_VAO);

    // 3. Subir datos crudos de los vértices al VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // 4. Subir datos de los índices al EBO (si la malla es indexada)
    if (isIndexed()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size() * sizeof(unsigned int),
                     indices.data(),
                     GL_STATIC_DRAW);
    }

    // 5. Configurar punteros de atributos de vértices (Vertex Attrib Pointers)
    
    // Atributo 0: Posición (vec3) -> layout (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

    // Atributo 1: Normal (vec3) -> layout (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Atributo 2: Coordenadas de Textura (vec2) -> layout (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // 6. Desenlazar el VAO para evitar modificaciones accidentales
    // NOTA: No desenlazar GL_ELEMENT_ARRAY_BUFFER antes de glBindVertexArray(0),
    // ya que el VAO guarda activamente la asociación al EBO.
    glBindVertexArray(0);

    // 7. Generar buffer de líneas de normales para inspección de depuración (REQ-A8)
    if (!vertices.empty()) {
        std::vector<glm::vec3> normalLines;
        normalLines.reserve(vertices.size() * 2);
        const float normalLength = 0.12f;

        for (const auto& v : vertices) {
            glm::vec3 n = v.Normal;
            float len = glm::length(n);
            if (len > 1e-5f) {
                n /= len;
            } else {
                n = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            // Cada normal se representa como un segmento desde el vértice hacia afuera
            normalLines.push_back(v.Position);
            normalLines.push_back(v.Position + n * normalLength);
        }

        m_normalsCount = normalLines.size();

        glGenVertexArrays(1, &m_normalsVAO);
        glGenBuffers(1, &m_normalsVBO);

        glBindVertexArray(m_normalsVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalsVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     normalLines.size() * sizeof(glm::vec3),
                     normalLines.data(),
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);
    }
}

void Mesh::draw(DrawMode mode) const {
    if (m_VAO == 0) return;

    // Configurar el modo de rasterizado del polígono
    switch (mode) {
        case DrawMode::Fill:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case DrawMode::Wireframe:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case DrawMode::Points:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
    }

    // Enlazar el VAO que contiene la configuración de buffers y atributos
    glBindVertexArray(m_VAO);

    // Renderizado según si la malla tiene índices o no
    if (isIndexed()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    }

    // Restaurar estado por defecto (buena práctica para no afectar otros renders)
    glBindVertexArray(0);
    if (mode != DrawMode::Fill) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Mesh::drawTriangle(unsigned int triangleIndex, DrawMode mode) const {
    if (m_VAO == 0) return;
    size_t totalTriangles = getTriangleCount();
    if (triangleIndex >= totalTriangles) return;

    switch (mode) {
        case DrawMode::Fill:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case DrawMode::Wireframe:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case DrawMode::Points:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
    }

    glBindVertexArray(m_VAO);

    if (isIndexed()) {
        const void* offset = reinterpret_cast<const void*>(static_cast<uintptr_t>(triangleIndex * 3 * sizeof(unsigned int)));
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, offset);
    } else {
        glDrawArrays(GL_TRIANGLES, static_cast<GLint>(triangleIndex * 3), 3);
    }

    glBindVertexArray(0);

    if (mode != DrawMode::Fill) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Mesh::drawNormals() const {
    if (m_normalsVAO == 0 || m_normalsCount == 0) return;
    glBindVertexArray(m_normalsVAO);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_normalsCount));
    glBindVertexArray(0);
}

void Mesh::cleanup() {
    if (m_normalsVBO != 0) {
        glDeleteBuffers(1, &m_normalsVBO);
        m_normalsVBO = 0;
    }
    if (m_normalsVAO != 0) {
        glDeleteVertexArrays(1, &m_normalsVAO);
        m_normalsVAO = 0;
    }
    m_normalsCount = 0;

    if (m_EBO != 0) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}
