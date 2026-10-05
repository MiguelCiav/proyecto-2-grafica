#include "scene/Scene.h"
#include "core/Shader.h"
#include "graphics/Framebuffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <limits>

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

Scene::~Scene() {
    if (m_debugLinesVBO != 0) {
        glDeleteBuffers(1, &m_debugLinesVBO);
        m_debugLinesVBO = 0;
    }
    if (m_debugLinesVAO != 0) {
        glDeleteVertexArrays(1, &m_debugLinesVAO);
        m_debugLinesVAO = 0;
    }
}

void Scene::initDebugBuffers() {
    if (m_debugLinesVAO == 0) {
        glGenVertexArrays(1, &m_debugLinesVAO);
        glGenBuffers(1, &m_debugLinesVBO);

        glBindVertexArray(m_debugLinesVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_debugLinesVBO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindVertexArray(0);
    }
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
    m_nextId = 1;
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

void Scene::renderDebug(Shader& debugShader, const glm::mat4& view, const glm::mat4& projection, float pointSize) {
    if (m_debugLinesVAO == 0) {
        initDebugBuffers();
    }

    debugShader.use();
    debugShader.setMat4("projection", projection);
    debugShader.setMat4("view", view);

    // Permitir que las geometrías de inspección coincidentes con la superficie pasen el depth test
    glDepthFunc(GL_LEQUAL);

    for (const auto& obj : m_objects) {
        if (!obj || !obj->visible || !obj->model) {
            continue;
        }

        glm::mat4 modelMat = obj->getModelMatrix();
        const auto& subMeshes = obj->model->getSubMeshes();

        // 1. Visualizar Normales Vectoriales (GL_LINES con debug.vert / debug.frag)
        if (obj->showNormals) {
            debugShader.setMat4("model", modelMat);
            debugShader.setVec4("debugColor", glm::vec4(0.0f, 0.9f, 1.0f, 1.0f)); // Cian brillante
            glLineWidth(2.0f);
            for (const auto& sm : subMeshes) {
                sm.mesh.drawNormals();
            }
            glLineWidth(1.0f);
        }

        // 2. Visualizar Nube de Puntos de Vértices (GL_POINTS con glPointSize configurable)
        if (obj->showVertices) {
            debugShader.setMat4("model", modelMat);
            debugShader.setVec4("debugColor", glm::vec4(1.0f, 0.95f, 0.15f, 1.0f)); // Amarillo brillante
            debugShader.setFloat("pointSize", pointSize);

            glPointSize(pointSize);
            glEnable(GL_PROGRAM_POINT_SIZE);

            for (const auto& sm : subMeshes) {
                sm.mesh.draw(DrawMode::Points);
            }

            glPointSize(1.0f);
            glDisable(GL_PROGRAM_POINT_SIZE);
        }

        // 3. Visualizar Bounding Box AABB en Wireframe (GL_LINES envolviendo solidariamente al objeto)
        if (obj->showBoundingBox) {
            glm::vec3 localMin = obj->model->getMinBound();
            glm::vec3 localMax = obj->model->getMaxBound();

            if (localMin.x > localMax.x) {
                localMin = glm::vec3(std::numeric_limits<float>::max());
                localMax = glm::vec3(-std::numeric_limits<float>::max());
                for (const auto& sm : subMeshes) {
                    for (const auto& v : sm.mesh.vertices) {
                        localMin = glm::min(localMin, v.Position);
                        localMax = glm::max(localMax, v.Position);
                    }
                }
            }

            // Los 8 vértices de la caja en coordenadas locales del objeto
            glm::vec3 corners[8] = {
                glm::vec3(localMin.x, localMin.y, localMin.z),
                glm::vec3(localMax.x, localMin.y, localMin.z),
                glm::vec3(localMax.x, localMax.y, localMin.z),
                glm::vec3(localMin.x, localMax.y, localMin.z),
                glm::vec3(localMin.x, localMin.y, localMax.z),
                glm::vec3(localMax.x, localMin.y, localMax.z),
                glm::vec3(localMax.x, localMax.y, localMax.z),
                glm::vec3(localMin.x, localMax.y, localMax.z)
            };

            // Transformar solidariamente las 8 esquinas con la matriz de modelo a coordenadas de mundo
            glm::vec3 worldMin(std::numeric_limits<float>::max());
            glm::vec3 worldMax(-std::numeric_limits<float>::max());
            for (int i = 0; i < 8; ++i) {
                glm::vec4 wc = modelMat * glm::vec4(corners[i], 1.0f);
                worldMin = glm::min(worldMin, glm::vec3(wc));
                worldMax = glm::max(worldMax, glm::vec3(wc));
            }

            // 8 esquinas alineadas a los ejes de mundo (AABB)
            glm::vec3 c[8] = {
                glm::vec3(worldMin.x, worldMin.y, worldMin.z), // 0: ---
                glm::vec3(worldMax.x, worldMin.y, worldMin.z), // 1: +--
                glm::vec3(worldMax.x, worldMax.y, worldMin.z), // 2: ++-
                glm::vec3(worldMin.x, worldMax.y, worldMin.z), // 3: -+-
                glm::vec3(worldMin.x, worldMin.y, worldMax.z), // 4: --+
                glm::vec3(worldMax.x, worldMin.y, worldMax.z), // 5: +-+
                glm::vec3(worldMax.x, worldMax.y, worldMax.z), // 6: +++
                glm::vec3(worldMin.x, worldMax.y, worldMax.z)  // 7: -++
            };

            // 12 aristas de alambre (Wireframe) representadas como segmentos de GL_LINES
            glm::vec3 aabbLines[24] = {
                c[0], c[1],  c[1], c[2],  c[2], c[3],  c[3], c[0], // Cara inferior (Z-)
                c[4], c[5],  c[5], c[6],  c[6], c[7],  c[7], c[4], // Cara superior (Z+)
                c[0], c[4],  c[1], c[5],  c[2], c[6],  c[3], c[7]  // Aristas verticales
            };

            debugShader.setMat4("model", glm::mat4(1.0f));
            debugShader.setVec4("debugColor", glm::vec4(0.15f, 1.0f, 0.35f, 1.0f)); // Verde lima brillante

            glBindVertexArray(m_debugLinesVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_debugLinesVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(aabbLines), aabbLines, GL_DYNAMIC_DRAW);

            glLineWidth(2.0f);
            glDrawArrays(GL_LINES, 0, 24);
            glLineWidth(1.0f);

            glBindVertexArray(0);
        }
    }

    // Restaurar función de prueba de profundidad por defecto
    glDepthFunc(GL_LESS);
}
