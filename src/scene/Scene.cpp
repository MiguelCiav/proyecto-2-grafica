#include "scene/Scene.h"
#include "core/Shader.h"
#include "graphics/Framebuffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

glm::mat4 Transform::getMatrix() const {
    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, position);
    mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::scale(mat, scale);
    return mat;
}

SceneObject::SceneObject(unsigned int id, const std::string& name, std::shared_ptr<Model> model)
    : id(id), name(name), model(std::move(model)) {
    if (this->model && !this->model->getSubMeshes().empty()) {
        color = this->model->getSubMeshes()[0].diffuseColor;
    }
}

Scene::Scene() {
    m_light.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    m_light.color = glm::vec3(1.0f, 1.0f, 1.0f);
    m_light.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
}

std::shared_ptr<SceneObject> Scene::addObject(const std::string& name, std::shared_ptr<Model> model) {
    auto obj = std::make_shared<SceneObject>(m_nextId++, name, std::move(model));
    m_objects.push_back(obj);
    return obj;
}

bool Scene::removeObject(unsigned int id) {
    auto it = std::remove_if(m_objects.begin(), m_objects.end(),
        [id](const std::shared_ptr<SceneObject>& obj) {
            return obj && obj->id == id;
        });

    if (it != m_objects.end()) {
        m_objects.erase(it, m_objects.end());
        return true;
    }
    return false;
}

std::shared_ptr<SceneObject> Scene::getObject(unsigned int id) {
    for (const auto& obj : m_objects) {
        if (obj && obj->id == id) {
            return obj;
        }
    }
    return nullptr;
}

void Scene::clear() {
    m_objects.clear();
    selectedObjectID = 0;
}

void Scene::update(float dt) {
    (void)dt;
    // Punto de extensión para animaciones, dinámicas temporales o transformaciones continuas
}

void Scene::render(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    shader.setVec3("lightDir", m_light.direction);
    shader.setVec3("lightColor", m_light.color);
    shader.setVec3("ambientLight", m_light.ambient);

    for (const auto& obj : m_objects) {
        if (!obj || !obj->visible || !obj->model) {
            continue;
        }

        shader.setMat4("model", obj->getModelMatrix());
        shader.setVec4("objectColor", obj->color);

        DrawMode mode = DrawMode::Fill;
        if (obj->showWireframe) {
            mode = DrawMode::Wireframe;
        } else if (obj->showVertices) {
            mode = DrawMode::Points;
        }

        obj->model->draw(mode);
    }
}

void Scene::renderForPicking(Shader& shader, const glm::mat4& view, const glm::mat4& projection, SelectionMode mode) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    if (mode == SelectionMode::Triangle) {
        m_triangleLookup.clear();
        shader.setInt("pickingMode", 1); // Modo Triángulo con gl_PrimitiveID

        int currentBaseID = 0;
        for (const auto& obj : m_objects) {
            if (!obj || !obj->visible || !obj->model) {
                continue;
            }

            shader.setMat4("model", obj->getModelMatrix());
            const auto& subMeshes = obj->model->getSubMeshes();
            for (size_t s = 0; s < subMeshes.size(); ++s) {
                const auto& sm = subMeshes[s];
                unsigned int triCount = static_cast<unsigned int>(sm.mesh.getTriangleCount());
                if (triCount == 0) continue;

                shader.setInt("baseTriangleID", currentBaseID);

                for (unsigned int t = 0; t < triCount; ++t) {
                    m_triangleLookup.push_back(TriangleHit{
                        obj->id,
                        static_cast<unsigned int>(s),
                        t
                    });
                }

                currentBaseID += static_cast<int>(triCount);
                sm.mesh.draw(DrawMode::Fill);
            }
        }
    } else {
        shader.setInt("pickingMode", 0); // Modo Global o Local con codeColor

        for (const auto& obj : m_objects) {
            if (!obj || !obj->visible || !obj->model) {
                continue;
            }

            shader.setMat4("model", obj->getModelMatrix());

            if (mode == SelectionMode::Global) {
                // En modo Global, todo el modelo se dibuja con el ID del objeto raíz
                glm::vec3 codeColor = Framebuffer::encodeID(obj->id);
                shader.setVec3("codeColor", codeColor);
                obj->model->draw(DrawMode::Fill);
            } else if (mode == SelectionMode::Local) {
                // En modo Local, cada sub-mallado individual se dibuja con su ID jerárquico diferenciado
                const auto& subMeshes = obj->model->getSubMeshes();
                for (size_t i = 0; i < subMeshes.size(); ++i) {
                    unsigned int localId = Framebuffer::encodeLocalID(obj->id, static_cast<unsigned int>(i));
                    glm::vec3 codeColor = Framebuffer::encodeID(localId);
                    shader.setVec3("codeColor", codeColor);
                    subMeshes[i].mesh.draw(DrawMode::Fill);
                }
            }
        }
    }
}

bool Scene::getTriangleHit(unsigned int globalTriangleId, TriangleHit& outHit) const {
    if (globalTriangleId == 0 || globalTriangleId > m_triangleLookup.size()) {
        return false;
    }
    outHit = m_triangleLookup[globalTriangleId - 1];
    return true;
}
