#pragma once

#include "Mesh.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

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

    // Carga de archivo .obj y su correspondiente .mtl
    bool loadFromFile(const std::string& filepath);

    // Dibuja todos los sub-mallados del modelo
    void draw(DrawMode mode = DrawMode::Fill) const;

    // Getters
    const std::vector<SubMesh>& getSubMeshes() const { return m_subMeshes; }
    std::vector<SubMesh>& getSubMeshes() { return m_subMeshes; }
    const std::string& getFilepath() const { return m_filepath; }
    bool isLoaded() const { return !m_subMeshes.empty(); }

private:
    std::vector<SubMesh> m_subMeshes;
    std::string m_filepath;
};
