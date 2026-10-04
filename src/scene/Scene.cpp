#include "scene/Scene.h"
#include "core/Shader.h"
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

        DrawMode mode = DrawMode::Fill;
        if (obj->showWireframe) {
            mode = DrawMode::Wireframe;
        } else if (obj->showVertices) {
            mode = DrawMode::Points;
        }

        const auto& subMeshes = obj->model->getSubMeshes();
        if (subMeshes.size() > 1) {
            for (const auto& subMesh : subMeshes) {
                glm::vec4 finalColor = subMesh.diffuseColor * obj->color;
                shader.setVec4("objectColor", finalColor);
                subMesh.mesh.draw(mode);
            }
        } else {
            shader.setVec4("objectColor", obj->color);
            obj->model->draw(mode);
        }
    }
}
