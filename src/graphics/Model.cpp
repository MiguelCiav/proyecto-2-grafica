#include "Model.h"
#include <tiny_obj_loader.h>
#include <filesystem>
#include <iostream>

Model::Model(const std::string& filepath) {
    loadFromFile(filepath);
}

bool Model::loadFromFile(const std::string& filepath) {
    m_filepath = filepath;
    m_subMeshes.clear();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Obtener la carpeta contenedora para buscar el .mtl en la misma ruta
    std::filesystem::path p(filepath);
    std::string baseDir = p.has_parent_path() ? (p.parent_path().string() + "/") : "";

    // Cargar con triangulación automática activada
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), baseDir.c_str(), true);

    if (!warn.empty()) {
        std::cout << "[TinyObjLoader WARN]: " << warn << std::endl;
    }

    if (!ret) {
        std::cerr << "[TinyObjLoader ERROR]: " << err << std::endl;
        return false;
    }

    // Color difuso de reserva por defecto 
    const glm::vec4 defaultColor(0.8f, 0.8f, 0.8f, 1.0f);

    // Procesar cada figura/grupo como un SubMesh independiente
    for (const auto& s : shapes) {
        std::vector<Vertex> meshVertices;
        std::vector<unsigned int> meshIndices;

        // Determinar el color difuso Kd del material para este sub-mallado
        glm::vec4 diffuseColor = defaultColor;
        if (!s.mesh.material_ids.empty() && s.mesh.material_ids[0] >= 0) {
            int matId = s.mesh.material_ids[0];
            if (matId < static_cast<int>(materials.size())) {
                const auto& mat = materials[matId];
                diffuseColor = glm::vec4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f);
            }
        }

        // Extraer vértices e índices del sub-mallado
        for (size_t f = 0; f < s.mesh.indices.size(); f++) {
            tinyobj::index_t idx = s.mesh.indices[f];

            Vertex vertex{};

            // Posición (aPos)
            vertex.Position = glm::vec3(
                attrib.vertices[3 * idx.vertex_index + 0],
                attrib.vertices[3 * idx.vertex_index + 1],
                attrib.vertices[3 * idx.vertex_index + 2]
            );

            // Normales (si están presentes en el archivo)
            if (idx.normal_index >= 0) {
                vertex.Normal = glm::vec3(
                    attrib.normals[3 * idx.normal_index + 0],
                    attrib.normals[3 * idx.normal_index + 1],
                    attrib.normals[3 * idx.normal_index + 2]
                );
            } else {
                vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            // Coordenadas de textura (UV)
            if (idx.texcoord_index >= 0) {
                vertex.TexCoords = glm::vec2(
                    attrib.texcoords[2 * idx.texcoord_index + 0],
                    attrib.texcoords[2 * idx.texcoord_index + 1]
                );
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            meshVertices.push_back(vertex);
            meshIndices.push_back(static_cast<unsigned int>(f));
        }

        // Nombre del sub-mallado (si está vacío, generamos uno por defecto)
        std::string subMeshName = s.name.empty() ? ("SubMesh_" + std::to_string(m_subMeshes.size())) : s.name;

        // Construir el SubMesh y guardarlo en la lista
        m_subMeshes.push_back(SubMesh{
            Mesh(meshVertices, meshIndices),
            diffuseColor,
            subMeshName
        });
    }

    std::cout << "[INFO] Modelo cargado: " << filepath << " (" << m_subMeshes.size() << " sub-mallados)" << std::endl;
    return true;
}

void Model::draw(DrawMode mode) const {
    for (const auto& subMesh : m_subMeshes) {
        subMesh.mesh.draw(mode);
    }
}
