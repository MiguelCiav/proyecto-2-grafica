#include "scene/Scene.h"
#include "core/Shader.h"
#include "graphics/Framebuffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <limits>

glm::mat4 Transform::getMatrix() const
{
    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, position);
    mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::scale(mat, scale);
    return mat;
}

SceneObject::SceneObject(unsigned int id, const std::string &name, std::shared_ptr<Model> model)
    : id(id), name(name), model(std::move(model))
{
    if (this->model && !this->model->getSubMeshes().empty())
    {
        if (this->model->getSubMeshes().size() == 1)
        {
            color = this->model->getSubMeshes()[0].diffuseColor;
        }
        else
        {
            color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

Scene::Scene()
{
    m_light.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    m_light.color = glm::vec3(1.0f, 1.0f, 1.0f);
    m_light.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
}

Scene::~Scene()
{
    if (m_debugLinesVBO != 0)
    {
        glDeleteBuffers(1, &m_debugLinesVBO);
        m_debugLinesVBO = 0;
    }
    if (m_debugLinesVAO != 0)
    {
        glDeleteVertexArrays(1, &m_debugLinesVAO);
        m_debugLinesVAO = 0;
    }
}

void Scene::initDebugBuffers()
{
    if (m_debugLinesVAO == 0)
    {
        glGenVertexArrays(1, &m_debugLinesVAO);
        glGenBuffers(1, &m_debugLinesVBO);

        glBindVertexArray(m_debugLinesVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_debugLinesVBO);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

        glBindVertexArray(0);
    }
}

std::shared_ptr<SceneObject> Scene::addObject(const std::string &name, std::shared_ptr<Model> model)
{
    auto obj = std::make_shared<SceneObject>(m_nextId++, name, std::move(model));
    m_objects.push_back(obj);
    return obj;
}

bool Scene::removeObject(unsigned int id)
{
    if (selectedObjectID == id)
    {
        selectObject(0);
    }

    auto it = std::remove_if(m_objects.begin(), m_objects.end(),
                             [id](const std::shared_ptr<SceneObject> &obj)
                             {
                                 return obj && obj->id == id;
                             });

    if (it != m_objects.end())
    {
        m_objects.erase(it, m_objects.end());
        return true;
    }
    return false;
}

std::shared_ptr<SceneObject> Scene::getObject(unsigned int id)
{
    for (const auto &obj : m_objects)
    {
        if (obj && obj->id == id)
        {
            return obj;
        }
    }
    return nullptr;
}

void Scene::selectObject(unsigned int newId)
{
    if (selectedObjectID != newId)
    {
        if (auto prevObj = getObject(selectedObjectID))
        {
            prevObj->showBoundingBox = false;
        }
        selectedObjectID = newId;
    }
    if (auto newObj = getObject(selectedObjectID))
    {
        newObj->showBoundingBox = true;
    }
}

void Scene::clear()
{
    selectObject(0);
    m_objects.clear();
    m_nextId = 1;
}

void Scene::update(float dt)
{
    (void)dt;
}

void Scene::render(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection)
{
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    shader.setVec3("lightDir", m_light.direction);
    shader.setVec3("lightColor", m_light.color);
    shader.setVec3("ambientLight", m_light.ambient);

    for (const auto &obj : m_objects)
    {
        if (!obj || !obj->visible || !obj->model)
        {
            continue;
        }

        shader.setMat4("model", obj->getModelMatrix());

        DrawMode mode = DrawMode::Fill;
        if (obj->showWireframe)
        {
            mode = DrawMode::Wireframe;
        }

        const auto &subMeshes = obj->model->getSubMeshes();
        for (const auto &sm : subMeshes)
        {
            glm::vec4 effectiveColor = (subMeshes.size() == 1)
                                           ? obj->color
                                           : (sm.diffuseColor * obj->color);
            shader.setVec4("objectColor", effectiveColor);
            sm.mesh.draw(mode);
        }
    }
}

void Scene::renderForPicking(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection, SelectionMode mode)
{
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    if (mode == SelectionMode::Triangle)
    {
        m_triangleLookup.clear();
        shader.setInt("pickingMode", 1);

        int currentBaseID = 0;
        for (const auto &obj : m_objects)
        {
            if (!obj || !obj->visible || !obj->model)
            {
                continue;
            }

            shader.setMat4("model", obj->getModelMatrix());
            const auto &subMeshes = obj->model->getSubMeshes();
            for (size_t s = 0; s < subMeshes.size(); ++s)
            {
                const auto &sm = subMeshes[s];
                unsigned int triCount = static_cast<unsigned int>(sm.mesh.getTriangleCount());
                if (triCount == 0)
                    continue;

                shader.setInt("baseTriangleID", currentBaseID);

                for (unsigned int t = 0; t < triCount; ++t)
                {
                    m_triangleLookup.push_back(TriangleHit{
                        obj->id,
                        static_cast<unsigned int>(s),
                        t});
                }

                currentBaseID += static_cast<int>(triCount);
                sm.mesh.draw(DrawMode::Fill);
            }
        }
    }
    else
    {
        shader.setInt("pickingMode", 0);

        for (const auto &obj : m_objects)
        {
            if (!obj || !obj->visible || !obj->model)
            {
                continue;
            }

            shader.setMat4("model", obj->getModelMatrix());

            if (mode == SelectionMode::Global)
            {

                glm::vec3 codeColor = Framebuffer::encodeID(obj->id);
                shader.setVec3("codeColor", codeColor);
                obj->model->draw(DrawMode::Fill);
            }
            else if (mode == SelectionMode::Local)
            {

                const auto &subMeshes = obj->model->getSubMeshes();
                for (size_t i = 0; i < subMeshes.size(); ++i)
                {
                    unsigned int localId = Framebuffer::encodeLocalID(obj->id, static_cast<unsigned int>(i));
                    glm::vec3 codeColor = Framebuffer::encodeID(localId);
                    shader.setVec3("codeColor", codeColor);
                    subMeshes[i].mesh.draw(DrawMode::Fill);
                }
            }
        }
    }
}

bool Scene::getTriangleHit(unsigned int globalTriangleId, TriangleHit &outHit) const
{
    if (globalTriangleId == 0 || globalTriangleId > m_triangleLookup.size())
    {
        return false;
    }
    outHit = m_triangleLookup[globalTriangleId - 1];
    return true;
}

void Scene::renderDebug(Shader &debugShader, const glm::mat4 &view, const glm::mat4 &projection, float pointSize)
{
    if (m_debugLinesVAO == 0)
    {
        initDebugBuffers();
    }

    debugShader.use();
    debugShader.setMat4("projection", projection);
    debugShader.setMat4("view", view);

    glDepthFunc(GL_LEQUAL);

    for (const auto &obj : m_objects)
    {
        if (!obj || !obj->visible || !obj->model)
        {
            continue;
        }

        glm::mat4 modelMat = obj->getModelMatrix();
        const auto &subMeshes = obj->model->getSubMeshes();

        if (obj->showNormals)
        {
            debugShader.setMat4("model", modelMat);
            debugShader.setVec4("debugColor", m_debugPalette.normals);
            glLineWidth(2.0f);
            for (const auto &sm : subMeshes)
            {
                sm.mesh.drawNormals();
            }
            glLineWidth(1.0f);
        }

        if (obj->showVertices)
        {
            debugShader.setMat4("model", modelMat);
            debugShader.setVec4("debugColor", m_debugPalette.vertices);
            debugShader.setFloat("pointSize", pointSize);

            glPointSize(pointSize);
            glEnable(GL_PROGRAM_POINT_SIZE);

            for (const auto &sm : subMeshes)
            {
                sm.mesh.draw(DrawMode::Points);
            }

            glPointSize(1.0f);
            glDisable(GL_PROGRAM_POINT_SIZE);
        }

        if (obj->showBoundingBox)
        {
            glm::vec3 localMin = obj->model->getMinBound();
            glm::vec3 localMax = obj->model->getMaxBound();

            if (localMin.x > localMax.x)
            {
                localMin = glm::vec3(std::numeric_limits<float>::max());
                localMax = glm::vec3(-std::numeric_limits<float>::max());
                for (const auto &sm : subMeshes)
                {
                    for (const auto &v : sm.mesh.vertices)
                    {
                        localMin = glm::min(localMin, v.Position);
                        localMax = glm::max(localMax, v.Position);
                    }
                }
            }

            glm::vec3 corners[8] = {
                glm::vec3(localMin.x, localMin.y, localMin.z),
                glm::vec3(localMax.x, localMin.y, localMin.z),
                glm::vec3(localMax.x, localMax.y, localMin.z),
                glm::vec3(localMin.x, localMax.y, localMin.z),
                glm::vec3(localMin.x, localMin.y, localMax.z),
                glm::vec3(localMax.x, localMin.y, localMax.z),
                glm::vec3(localMax.x, localMax.y, localMax.z),
                glm::vec3(localMin.x, localMax.y, localMax.z)};

            glm::vec3 worldMin(std::numeric_limits<float>::max());
            glm::vec3 worldMax(-std::numeric_limits<float>::max());
            for (int i = 0; i < 8; ++i)
            {
                glm::vec4 wc = modelMat * glm::vec4(corners[i], 1.0f);
                worldMin = glm::min(worldMin, glm::vec3(wc));
                worldMax = glm::max(worldMax, glm::vec3(wc));
            }

            glm::vec3 c[8] = {
                glm::vec3(worldMin.x, worldMin.y, worldMin.z),
                glm::vec3(worldMax.x, worldMin.y, worldMin.z),
                glm::vec3(worldMax.x, worldMax.y, worldMin.z),
                glm::vec3(worldMin.x, worldMax.y, worldMin.z),
                glm::vec3(worldMin.x, worldMin.y, worldMax.z),
                glm::vec3(worldMax.x, worldMin.y, worldMax.z),
                glm::vec3(worldMax.x, worldMax.y, worldMax.z),
                glm::vec3(worldMin.x, worldMax.y, worldMax.z)};

            glm::vec3 aabbLines[24] = {
                c[0], c[1], c[1], c[2], c[2], c[3], c[3], c[0],
                c[4], c[5], c[5], c[6], c[6], c[7], c[7], c[4],
                c[0], c[4], c[1], c[5], c[2], c[6], c[3], c[7]};

            debugShader.setMat4("model", glm::mat4(1.0f));
            debugShader.setVec4("debugColor", m_debugPalette.boundingBox);

            glBindVertexArray(m_debugLinesVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_debugLinesVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(aabbLines), aabbLines, GL_DYNAMIC_DRAW);

            glLineWidth(2.0f);
            glDrawArrays(GL_LINES, 0, 24);
            glLineWidth(1.0f);

            glBindVertexArray(0);
        }
    }

    glDepthFunc(GL_LESS);
}
