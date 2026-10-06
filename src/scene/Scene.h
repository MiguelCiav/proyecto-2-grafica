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
struct Transform
{
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    glm::mat4 getMatrix() const;
};

/**
 * @brief Entidad presente en la escena 3D.
 */
class SceneObject
{
public:
    unsigned int id{0};
    std::string name{"Objeto"};
    Transform transform;
    glm::vec4 color{0.8f, 0.8f, 0.8f, 1.0f};

    std::shared_ptr<Model> model{nullptr};

    bool visible{true};
    bool showWireframe{false};
    bool showNormals{false};
    bool showVertices{false};
    bool showBoundingBox{false};

    SceneObject(unsigned int id, const std::string &name, std::shared_ptr<Model> model);

    glm::mat4 getModelMatrix() const { return transform.getMatrix(); }
};

/**
 * @brief Parámetros de iluminación direccional global.
 */
struct DirectionalLight
{
    glm::vec3 direction{-0.2f, -1.0f, -0.3f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    glm::vec3 ambient{0.2f, 0.2f, 0.2f};
};

/**
 * @brief Modos de especificidad para selección interactiva por Color Picking.
 */
enum class SelectionMode
{
    Global,
    Local,
    Triangle
};

/**
 * @brief Estructura de mapeo inverso para decodificación de clics en modo Triángulo.
 */
struct TriangleHit
{
    unsigned int objectId{0};
    unsigned int subMeshIndex{0};
    unsigned int localTriangleIndex{0};
};

/**
 * @brief Paleta de colores armónica para herramientas de inspección geométrica.
 */
struct DebugPalette
{
    glm::vec4 normals{0.12f, 0.65f, 0.95f, 1.0f};
    glm::vec4 vertices{1.0f, 0.62f, 0.10f, 1.0f};
    glm::vec4 boundingBox{0.22f, 0.85f, 0.52f, 1.0f};
};

/**
 * @brief Gestor principal de la escena 3D.
 */
class Scene
{
public:
    Scene();
    ~Scene();

    std::shared_ptr<SceneObject> addObject(const std::string &name, std::shared_ptr<Model> model);
    bool removeObject(unsigned int id);
    std::shared_ptr<SceneObject> getObject(unsigned int id);
    const std::vector<std::shared_ptr<SceneObject>> &getObjects() const { return m_objects; }

    void clear();

    DirectionalLight &getLight() { return m_light; }
    const DirectionalLight &getLight() const { return m_light; }

    glm::vec4 &getBackgroundColor() { return m_backgroundColor; }
    const glm::vec4 &getBackgroundColor() const { return m_backgroundColor; }

    DebugPalette &getDebugPalette() { return m_debugPalette; }
    const DebugPalette &getDebugPalette() const { return m_debugPalette; }

    void update(float dt);

    unsigned int selectedObjectID{0};
    unsigned int getSelectedObjectId() const { return selectedObjectID; }
    void setSelectedObjectId(unsigned int id) { selectObject(id); }
    void selectObject(unsigned int id);

    void render(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection);

    void renderForPicking(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection, SelectionMode mode);

    void renderDebug(Shader &debugShader, const glm::mat4 &view, const glm::mat4 &projection, float pointSize = 6.0f);

    bool getTriangleHit(unsigned int globalTriangleId, TriangleHit &outHit) const;

private:
    std::vector<std::shared_ptr<SceneObject>> m_objects;
    unsigned int m_nextId{1};
    DirectionalLight m_light;
    glm::vec4 m_backgroundColor{0.12f, 0.12f, 0.14f, 1.0f};
    DebugPalette m_debugPalette;

    std::vector<TriangleHit> m_triangleLookup;

    unsigned int m_debugLinesVAO{0};
    unsigned int m_debugLinesVBO{0};
    void initDebugBuffers();
};
