#pragma once

#include "Mesh.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>


namespace tinyobj {
    struct attrib_t;
    struct shape_t;
}

/**
 * @brief Representa un sub-mallado individual con su propio material Kd.
 */
struct SubMesh {
    Mesh mesh;
    glm::vec4 diffuseColor{0.8f, 0.8f, 0.8f, 1.0f}; // Color difuso Kd (RGB) + Alfa (A)
    std::string name;
};

/**
 * @brief Gestiona la carga y almacenamiento de modelos 3D compuestos por uno o múltiples sub-mallados.
 */

class Model {
public:
    Model() = default;
    explicit Model(const std::string& filepath);
    Model(Mesh mesh, const std::string& name = "Primitive", const glm::vec4& diffuseColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    Model(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices = {}, const std::string& name = "Primitive", const glm::vec4& diffuseColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

    // Carga de archivo .obj y su correspondiente .mtl
    bool loadFromFile(const std::string& filepath);

    // Dibuja todos los sub-mallados del modelo
    void draw(DrawMode mode = DrawMode::Fill) const;

    // Getters
    const std::vector<SubMesh>& getSubMeshes() const { return m_subMeshes; }
    std::vector<SubMesh>& getSubMeshes() { return m_subMeshes; }
    const std::string& getFilepath() const { return m_filepath; }
    bool isLoaded() const { return !m_subMeshes.empty(); }

    glm::vec3 getMinBound() const { return m_minBound; }
    glm::vec3 getMaxBound() const { return m_maxBound; }

private:
    std::vector<SubMesh> m_subMeshes;
    std::string m_filepath;

    glm::vec3 m_minBound{-1.0f};
    glm::vec3 m_maxBound{1.0f};
    void normalizeModel(tinyobj::attrib_t& attrib);
    std::vector<glm::vec3> computeAverageNormals(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes);
};
