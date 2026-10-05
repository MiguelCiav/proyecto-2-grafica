#include "Model.h"
#include <tiny_obj_loader.h>
#include <filesystem>
#include <iostream>
#include <limits>
#include <algorithm>

Model::Model(const std::string& filepath) {
    loadFromFile(filepath);
}


void Model::normalizeModel(tinyobj::attrib_t& attrib){
    
    if(attrib.vertices.empty()) return;

    glm::vec3 minBound(std::numeric_limits<float>::max());
    glm::vec3 maxBound(-std::numeric_limits<float>::max());

    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        glm::vec3 pos(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
        minBound = glm::min(minBound, pos);
        maxBound = glm::max(maxBound, pos);
    }

    glm::vec3 center = (minBound + maxBound) * 0.5f;
    glm::vec3 extents = maxBound - minBound;
    float maxDim = std::max(extents.x, std::max(extents.y, extents.z));
    float scale = (maxDim > 1e-6f) ? (2.0f / maxDim) : 1.0f;

    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        attrib.vertices[i] = (attrib.vertices[i] - center.x) * scale;
        attrib.vertices[i + 1] = (attrib.vertices[i + 1] - center.y) * scale;
        attrib.vertices[i + 2] = (attrib.vertices[i + 2] - center.z) * scale;
    }

    m_minBound = (minBound - center) * scale;
    m_maxBound = (maxBound - center) * scale;
}

std::vector<glm::vec3> Model::computeAverageNormals(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes){
    
    size_t numPositions = attrib.vertices.size()/3;
    std::vector<glm::vec3> computedNormals(numPositions, glm::vec3(0.0f));

    for (const auto& s : shapes) {
        for (size_t f = 0; f < s.mesh.indices.size(); f += 3) {
            int i0 = s.mesh.indices[f + 0].vertex_index;
            int i1 = s.mesh.indices[f + 1].vertex_index;
            int i2 = s.mesh.indices[f + 2].vertex_index;
            if (i0 < 0 || i1 < 0 || i2 < 0) continue;
            glm::vec3 v0(attrib.vertices[3 * i0 + 0], attrib.vertices[3 * i0 + 1], attrib.vertices[3 * i0 + 2]);
            glm::vec3 v1(attrib.vertices[3 * i1 + 0], attrib.vertices[3 * i1 + 1], attrib.vertices[3 * i1 + 2]);
            glm::vec3 v2(attrib.vertices[3 * i2 + 0], attrib.vertices[3 * i2 + 1], attrib.vertices[3 * i2 + 2]);
            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 faceNormal = glm::cross(edge1, edge2);
            computedNormals[i0] += faceNormal;
            computedNormals[i1] += faceNormal;
            computedNormals[i2] += faceNormal;
        }
    }

    for (size_t i = 0; i < computedNormals.size(); ++i) {
        float len = glm::length(computedNormals[i]);
        if (len > 1e-6f) {
            computedNormals[i] /= len;
        } else {
            computedNormals[i] = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    return computedNormals;
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

    // Normalizar modelo espacialmente (Centrado y acotado a [-1, 1])
    normalizeModel(attrib);

    // Comprobar si faltan normales y calcular normales promedio si es necesario
    bool hasNormals = !attrib.normals.empty();
    std::vector<glm::vec3> avgNormals;
    if (!hasNormals) {
        avgNormals = computeAverageNormals(attrib, shapes);
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
                vertex.Normal = avgNormals[idx.vertex_index];
            }

            // Asegurar que la normal sea unitaria
            if (glm::length(vertex.Normal) > 1e-6f) {
                vertex.Normal = glm::normalize(vertex.Normal);
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
