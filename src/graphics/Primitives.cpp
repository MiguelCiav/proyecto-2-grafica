#include "graphics/Primitives.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>

namespace Primitives {

Mesh createCubeMesh(float size) {
    float h = size * 0.5f;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(24);
    indices.reserve(36);

    // Función auxiliar para agregar una cara cuadrada con su normal analítica exacta
    auto addFace = [&](const glm::vec3& v0, const glm::vec3& v1,
                       const glm::vec3& v2, const glm::vec3& v3,
                       const glm::vec3& normal) {
        unsigned int baseIdx = static_cast<unsigned int>(vertices.size());

        vertices.push_back(Vertex{v0, normal, glm::vec2(0.0f, 0.0f)});
        vertices.push_back(Vertex{v1, normal, glm::vec2(1.0f, 0.0f)});
        vertices.push_back(Vertex{v2, normal, glm::vec2(1.0f, 1.0f)});
        vertices.push_back(Vertex{v3, normal, glm::vec2(0.0f, 1.0f)});

        // Dos triángulos en sentido antihorario (CCW)
        indices.push_back(baseIdx );
        indices.push_back(baseIdx + 1);
        indices.push_back(baseIdx + 2);

        indices.push_back(baseIdx );
        indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 3);
    };

    // 1. Cara Frontal (+Z)
    addFace(glm::vec3(-h, -h,  h), glm::vec3( h, -h,  h),
            glm::vec3( h,  h,  h), glm::vec3(-h,  h,  h),
            glm::vec3(0.0f, 0.0f, 1.0f));

    // 2. Cara Trasera (-Z)
    addFace(glm::vec3( h, -h, -h), glm::vec3(-h, -h, -h),
            glm::vec3(-h,  h, -h), glm::vec3( h,  h, -h),
            glm::vec3(0.0f, 0.0f, -1.0f));

    // 3. Cara Superior (+Y)
    addFace(glm::vec3(-h,  h,  h), glm::vec3( h,  h,  h),
            glm::vec3( h,  h, -h), glm::vec3(-h,  h, -h),
            glm::vec3(0.0f, 1.0f, 0.0f));

    // 4. Cara Inferior (-Y)
    addFace(glm::vec3(-h, -h, -h), glm::vec3( h, -h, -h),
            glm::vec3( h, -h,  h), glm::vec3(-h, -h,  h),
            glm::vec3(0.0f, -1.0f, 0.0f));

    // 5. Cara Derecha (+X)
    addFace(glm::vec3( h, -h,  h), glm::vec3( h, -h, -h),
            glm::vec3( h,  h, -h), glm::vec3( h,  h,  h),
            glm::vec3(1.0f, 0.0f, 0.0f));

    // 6. Cara Izquierda (-X)
    addFace(glm::vec3(-h, -h, -h), glm::vec3(-h, -h,  h),
            glm::vec3(-h,  h,  h), glm::vec3(-h,  h, -h),
            glm::vec3(-1.0f, 0.0f, 0.0f));

    return Mesh(vertices, indices);
}

Mesh createPyramidMesh(float base, float height) {
    float hb = base * 0.5f;
    float hh = height * 0.5f;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(16);
    indices.reserve(18);

    // Ápice superior de la pirámide
    glm::vec3 apex(0.0f, hh, 0.0f);

    // Esquinas de la base cuadrada en Y = -hh
    glm::vec3 bFrontLeft(-hb, -hh,  hb);
    glm::vec3 bFrontRight( hb, -hh,  hb);
    glm::vec3 bBackRight( hb, -hh, -hb);
    glm::vec3 bBackLeft(-hb, -hh, -hb);

    // 1. Base cuadrada inferior (Normal analítica: 0, -1, 0)
    glm::vec3 downNormal(0.0f, -1.0f, 0.0f);
    unsigned int baseStart = static_cast<unsigned int>(vertices.size());
    vertices.push_back(Vertex{bBackLeft,   downNormal, glm::vec2(0.0f, 0.0f)});
    vertices.push_back(Vertex{bBackRight,  downNormal, glm::vec2(1.0f, 0.0f)});
    vertices.push_back(Vertex{bFrontRight, downNormal, glm::vec2(1.0f, 1.0f)});
    vertices.push_back(Vertex{bFrontLeft,  downNormal, glm::vec2(0.0f, 1.0f)});

    indices.push_back(baseStart );
    indices.push_back(baseStart + 1);
    indices.push_back(baseStart + 2);

    indices.push_back(baseStart );
    indices.push_back(baseStart + 2);
    indices.push_back(baseStart + 3);

    // Función auxiliar para agregar cara lateral triangular con normal analítica exacta
    auto addSideFace = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(e1, e2));

        unsigned int idx = static_cast<unsigned int>(vertices.size());
        vertices.push_back(Vertex{v0, normal, glm::vec2(0.0f, 0.0f)});
        vertices.push_back(Vertex{v1, normal, glm::vec2(1.0f, 0.0f)});
        vertices.push_back(Vertex{v2, normal, glm::vec2(0.5f, 1.0f)});

        indices.push_back(idx );
        indices.push_back(idx + 1);
        indices.push_back(idx + 2);
    };

    // 2. Cara Frontal (+Z)
    addSideFace(bFrontLeft, bFrontRight, apex);

    // 3. Cara Derecha (+X)
    addSideFace(bFrontRight, bBackRight, apex);

    // 4. Cara Trasera (-Z)
    addSideFace(bBackRight, bBackLeft, apex);

    // 5. Cara Izquierda (-X)
    addSideFace(bBackLeft, bFrontLeft, apex);

    return Mesh(vertices, indices);
}

Mesh createSphereMesh(float radius, unsigned int sectors, unsigned int stacks) {
    sectors = std::max(sectors, 3u);
    stacks = std::max(stacks, 2u);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve((stacks + 1) * (sectors + 1));
    indices.reserve(stacks * sectors * 6);

    const float pi = glm::pi<float>();

    for (unsigned int i = 0; i <= stacks; ++i) {
        // Latitud: de pi/2 (polo norte) a -pi/2 (polo sur)
        float stackAngle = (pi / 2.0f) - static_cast<float>(i) * (pi / static_cast<float>(stacks));
        float xy = radius * std::cos(stackAngle);
        float y  = radius * std::sin(stackAngle);

        for (unsigned int j = 0; j <= sectors; ++j) {
            // Longitud: de 0 a 2*pi
            float sectorAngle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectors));
            float x = xy * std::cos(sectorAngle);
            float z = xy * std::sin(sectorAngle);

            glm::vec3 pos(x, y, z);
            // Normal analítica radial exacta
            glm::vec3 normal = (radius > 1e-6f) ? (pos / radius) : glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec2 uv(
                static_cast<float>(j) / static_cast<float>(sectors),
                static_cast<float>(i) / static_cast<float>(stacks)
            );

            vertices.push_back(Vertex{pos, normal, uv});
        }
    }

    // Generar índices en sentido CCW
    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + sectors + 1;

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // Primer triángulo (omitir en polo norte donde k1 colapsa)
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k1 + 1);
                indices.push_back(k2);
            }

            // Segundo triángulo (omitir en polo sur donde k2 colapsa)
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2 + 1);
                indices.push_back(k2);
            }
        }
    }

    return Mesh(vertices, indices);
}

Mesh createCylinderMesh(float radius, float height, unsigned int sectors) {
    sectors = std::max(sectors, 3u);
    float hh = height * 0.5f;
    const float pi = glm::pi<float>();

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // --- 1. Manto Lateral ---
    unsigned int sideStartIdx = static_cast<unsigned int>(vertices.size());
    for (unsigned int j = 0; j <= sectors; ++j) {
        float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectors));
        float c = std::cos(angle);
        float s = std::sin(angle);

        float x = radius * c;
        float z = radius * s;
        // Normal analítica perpendicular al eje vertical Y
        glm::vec3 sideNormal(c, 0.0f, s);
        float u = static_cast<float>(j) / static_cast<float>(sectors);

        // Vértice superior
        vertices.push_back(Vertex{glm::vec3(x,  hh, z), sideNormal, glm::vec2(u, 1.0f)});
        // Vértice inferior
        vertices.push_back(Vertex{glm::vec3(x, -hh, z), sideNormal, glm::vec2(u, 0.0f)});
    }

    for (unsigned int j = 0; j < sectors; ++j) {
        unsigned int top1 = sideStartIdx + j * 2;
        unsigned int bot1 = top1 + 1;
        unsigned int top2 = sideStartIdx + (j + 1) * 2;
        unsigned int bot2 = top2 + 1;

        // Triángulo 1 (CCW)
        indices.push_back(top1);
        indices.push_back(top2);
        indices.push_back(bot1);

        // Triángulo 2 (CCW)
        indices.push_back(top2);
        indices.push_back(bot2);
        indices.push_back(bot1);
    }

    // --- 2. Tapa Superior (+Y) ---
    unsigned int topCapCenterIdx = static_cast<unsigned int>(vertices.size());
    glm::vec3 topNormal(0.0f, 1.0f, 0.0f);
    vertices.push_back(Vertex{glm::vec3(0.0f, hh, 0.0f), topNormal, glm::vec2(0.5f, 0.5f)});

    unsigned int topCapRimStart = static_cast<unsigned int>(vertices.size());
    for (unsigned int j = 0; j <= sectors; ++j) {
        float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectors));
        float c = std::cos(angle);
        float s = std::sin(angle);
        vertices.push_back(Vertex{
            glm::vec3(radius * c, hh, radius * s),
            topNormal,
            glm::vec2(0.5f + 0.5f * c, 0.5f + 0.5f * s)
        });
    }

    for (unsigned int j = 0; j < sectors; ++j) {
        indices.push_back(topCapCenterIdx);
        indices.push_back(topCapRimStart + j + 1);
        indices.push_back(topCapRimStart + j);
    }

    // --- 3. Tapa Inferior (-Y) ---
    unsigned int botCapCenterIdx = static_cast<unsigned int>(vertices.size());
    glm::vec3 botNormal(0.0f, -1.0f, 0.0f);
    vertices.push_back(Vertex{glm::vec3(0.0f, -hh, 0.0f), botNormal, glm::vec2(0.5f, 0.5f)});

    unsigned int botCapRimStart = static_cast<unsigned int>(vertices.size());
    for (unsigned int j = 0; j <= sectors; ++j) {
        float angle = static_cast<float>(j) * (2.0f * pi / static_cast<float>(sectors));
        float c = std::cos(angle);
        float s = std::sin(angle);
        vertices.push_back(Vertex{
            glm::vec3(radius * c, -hh, radius * s),
            botNormal,
            glm::vec2(0.5f + 0.5f * c, 0.5f + 0.5f * s)
        });
    }

    for (unsigned int j = 0; j < sectors; ++j) {
        indices.push_back(botCapCenterIdx);
        indices.push_back(botCapRimStart + j);
        indices.push_back(botCapRimStart + j + 1);
    }

    return Mesh(vertices, indices);
}

// --- Generación de Modelos compartidos para SceneObject ---

std::shared_ptr<Model> createCube(float size) {
    return std::make_shared<Model>(createCubeMesh(size), "Cubo");
}

std::shared_ptr<Model> createPyramid(float base, float height) {
    return std::make_shared<Model>(createPyramidMesh(base, height), "Piramide");
}

std::shared_ptr<Model> createSphere(float radius, unsigned int sectors, unsigned int stacks) {
    return std::make_shared<Model>(createSphereMesh(radius, sectors, stacks), "Esfera");
}

std::shared_ptr<Model> createCylinder(float radius, float height, unsigned int sectors) {
    return std::make_shared<Model>(createCylinderMesh(radius, height, sectors), "Cilindro");
}

} // namespace Primitives
