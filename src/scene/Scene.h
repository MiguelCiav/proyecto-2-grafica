#pragma once

#include "graphics/Model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <memory>

class Shader;

/**
 * @brief Almacena las componentes de transformación espacial de una entidad.
 */
struct Transform {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f}; // Ángulos de Euler en grados (X, Y, Z)
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    glm::mat4 getMatrix() const;
};

/**
 * @brief Entidad presente en la escena 3D.
 */
class SceneObject {
public:
    unsigned int id{0};
    std::string name{"Objeto"};
    Transform transform;
    glm::vec4 color{0.8f, 0.8f, 0.8f, 1.0f}; // Color difuso (RGB) y canal alfa (A)

    std::shared_ptr<Model> model{nullptr};

    // Banderas de inspección geométrica
    bool visible{true};
    bool showWireframe{false};
    bool showNormals{false};
    bool showVertices{false};
    bool showBoundingBox{false};

    SceneObject(unsigned int id, const std::string& name, std::shared_ptr<Model> model);

    glm::mat4 getModelMatrix() const { return transform.getMatrix(); }
};

/**
 * @brief Parámetros de iluminación direccional global.
 */
struct DirectionalLight {
    glm::vec3 direction{-0.2f, -1.0f, -0.3f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    glm::vec3 ambient{0.2f, 0.2f, 0.2f};
};

/**
 * @brief Modos de especificidad para selección interactiva por Color Picking.
 */
enum class SelectionMode {
    Global,     // Objeto completo (nodo raíz)
    Local,      // Sub-mallado individual (Mesh)
    Triangle    // Triángulo específico (Requisito Parejas)
};

/**
 * @brief Estructura de mapeo inverso para decodificación de clics en modo Triángulo.
 */
struct TriangleHit {
    unsigned int objectId{0};
    unsigned int subMeshIndex{0};
    unsigned int localTriangleIndex{0};
};

/**
 * @brief Paleta de colores armónica para herramientas de inspección geométrica.
 */
struct DebugPalette {
    glm::vec4 normals{0.12f, 0.65f, 0.95f, 1.0f};     // Azul cian tecnológico (#1FA6F2)
    glm::vec4 vertices{1.0f, 0.62f, 0.10f, 1.0f};     // Ámbar dorado cálido (#FF9E1A)
    glm::vec4 boundingBox{0.22f, 0.85f, 0.52f, 1.0f}; // Verde menta técnico CAD (#38D985)
};

/**
 * @brief Gestor principal de la escena 3D.
 */
class Scene {
public:
    Scene();
    ~Scene();

    // Gestión de entidades
    std::shared_ptr<SceneObject> addObject(const std::string& name, std::shared_ptr<Model> model);
    bool removeObject(unsigned int id);
    std::shared_ptr<SceneObject> getObject(unsigned int id);
    const std::vector<std::shared_ptr<SceneObject>>& getObjects() const { return m_objects; }

    // Borrado total de la escena
    void clear();

    // Iluminación y entorno
    DirectionalLight& getLight() { return m_light; }
    const DirectionalLight& getLight() const { return m_light; }

    glm::vec4& getBackgroundColor() { return m_backgroundColor; }
    const glm::vec4& getBackgroundColor() const { return m_backgroundColor; }

    // Paleta de colores de inspección geométrica
    DebugPalette& getDebugPalette() { return m_debugPalette; }
    const DebugPalette& getDebugPalette() const { return m_debugPalette; }

    // Actualización de estado y lógica temporal de la escena
    void update(float dt);

    // Entidad seleccionada
    unsigned int selectedObjectID{0};
    unsigned int getSelectedObjectId() const { return selectedObjectID; }
    void setSelectedObjectId(unsigned int id) { selectedObjectID = id; }

    // Renderizado de todos los objetos activos
    void render(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

    // Renderizado para selección por color (Color Picking en FBO)
    void renderForPicking(Shader& shader, const glm::mat4& view, const glm::mat4& projection, SelectionMode mode);

    // Renderizado de herramientas de inspección geométrica (Normales, Vértices, Bounding Box) (REQ-A8)
    void renderDebug(Shader& debugShader, const glm::mat4& view, const glm::mat4& projection, float pointSize = 6.0f);

    // Consulta de correspondencia de triángulo por su ID global de picking
    bool getTriangleHit(unsigned int globalTriangleId, TriangleHit& outHit) const;

private:
    std::vector<std::shared_ptr<SceneObject>> m_objects;
    unsigned int m_nextId{1}; // Asignación correlativa de IDs unívocos
    DirectionalLight m_light;
    glm::vec4 m_backgroundColor{0.12f, 0.12f, 0.14f, 1.0f};
    DebugPalette m_debugPalette;

    // Tabla de correspondencia para selección en Modo Triángulo
    std::vector<TriangleHit> m_triangleLookup;

    // Buffers dinámicos de OpenGL para dibujo de líneas de depuración (AABB)
    unsigned int m_debugLinesVAO{0};
    unsigned int m_debugLinesVBO{0};
    void initDebugBuffers();
};
