#include "scene/SceneSerializer.h"
#include "scene/Scene.h"
#include "graphics/Model.h"
#include "graphics/Primitives.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <unordered_map>
#include <algorithm>

SceneSerializer::SceneSerializer(Scene& scene)
    : m_scene(scene) {
}

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

bool SceneSerializer::serialize(const std::string& filepath) {
    try {
        std::filesystem::path p(filepath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    } catch (const std::exception& e) {
        m_lastError = "Error al crear directorios para: " + filepath + " (" + e.what() + ")";
        std::cerr << "[SceneSerializer ERROR]: " << m_lastError << std::endl;
        return false;
    }

    std::ofstream out(filepath);
    if (!out.is_open()) {
        m_lastError = "No se pudo abrir el archivo para escritura: " + filepath;
        std::cerr << "[SceneSerializer ERROR]: " << m_lastError << std::endl;
        return false;
    }

    out << std::fixed << std::setprecision(4);

    // 1. Cabecera y versión del formato de escena
    out << "# Proyecto Grafica UCV - Archivo de Escena 3D\n";
    out << "version: 1.0\n\n";

    // 2. Parámetros de entorno e iluminación global
    const auto& bg = m_scene.getBackgroundColor();
    const auto& light = m_scene.getLight();

    out << "environment:\n";
    out << "  bgColor: " << bg.r << " " << bg.g << " " << bg.b << " " << bg.a << "\n";
    out << "  lightDir: " << light.direction.x << " " << light.direction.y << " " << light.direction.z << "\n";
    out << "  lightColor: " << light.color.r << " " << light.color.g << " " << light.color.b << "\n";
    out << "  lightAmbient: " << light.ambient.r << " " << light.ambient.g << " " << light.ambient.b << "\n\n";

    // 3. Serialización de entidades de la escena
    const auto& objects = m_scene.getObjects();
    out << "objects: " << objects.size() << "\n";

    for (const auto& obj : objects) {
        if (!obj) continue;

        out << "  - object:\n";
        out << "      id: " << obj->id << "\n";
        out << "      name: " << obj->name << "\n";

        // Identificar origen del modelo: archivo OBJ o primitiva procedimental
        std::string modelSource = (obj->model) ? obj->model->getFilepath() : "assets/models/cube.obj";
        out << "      model: " << modelSource << "\n";

        // Transformaciones geométricas
        out << "      position: " << obj->transform.position.x << " " 
                                  << obj->transform.position.y << " " 
                                  << obj->transform.position.z << "\n";
        out << "      rotation: " << obj->transform.rotation.x << " " 
                                  << obj->transform.rotation.y << " " 
                                  << obj->transform.rotation.z << "\n";
        out << "      scale: "    << obj->transform.scale.x << " " 
                                  << obj->transform.scale.y << " " 
                                  << obj->transform.scale.z << "\n";

        // Material y canal alfa
        out << "      color: " << obj->color.r << " " 
                               << obj->color.g << " " 
                               << obj->color.b << " " 
                               << obj->color.a << "\n";

        // Banderas de estado e inspección geométrica
        out << "      visible: "     << (obj->visible ? 1 : 0) << "\n";
        out << "      wireframe: "   << (obj->showWireframe ? 1 : 0) << "\n";
        out << "      vertices: "    << (obj->showVertices ? 1 : 0) << "\n";
        out << "      normals: "     << (obj->showNormals ? 1 : 0) << "\n";
        out << "      boundingbox: " << (obj->showBoundingBox ? 1 : 0) << "\n";
    }

    out.close();
    std::cout << "[SceneSerializer INFO]: Escena guardada exitosamente en: " << filepath 
              << " (" << objects.size() << " objetos)" << std::endl;
    return true;
}

struct SerializedObjectData {
    unsigned int id{0};
    std::string name{"Objeto"};
    std::string modelPath{"assets/models/cube.obj"};
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec4 color{0.8f, 0.8f, 0.8f, 1.0f};
    bool visible{true};
    bool wireframe{false};
    bool vertices{false};
    bool normals{false};
    bool bbox{false};
};

bool SceneSerializer::deserialize(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) {
        m_lastError = "No se pudo abrir el archivo para lectura: " + filepath;
        std::cerr << "[SceneSerializer ERROR]: " << m_lastError << std::endl;
        return false;
    }

    glm::vec4 loadedBgColor{0.12f, 0.12f, 0.14f, 1.0f};
    DirectionalLight loadedLight;
    loadedLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    loadedLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    loadedLight.ambient = glm::vec3(0.2f, 0.2f, 0.2f);

    std::vector<SerializedObjectData> loadedObjects;
    SerializedObjectData currentObj;
    bool inObjectBlock = false;

    auto finishCurrentObject = [&]() {
        if (inObjectBlock) {
            loadedObjects.push_back(currentObj);
            currentObj = SerializedObjectData{};
            inObjectBlock = false;
        }
    };

    std::string line;
    while (std::getline(in, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue; // Saltar líneas vacías o comentarios
        }

        std::istringstream ss(trimmed);
        std::string key;
        ss >> key;

        // Entorno e iluminación
        if (key == "bgColor:") {
            ss >> loadedBgColor.r >> loadedBgColor.g >> loadedBgColor.b >> loadedBgColor.a;
        } else if (key == "lightDir:") {
            ss >> loadedLight.direction.x >> loadedLight.direction.y >> loadedLight.direction.z;
        } else if (key == "lightColor:") {
            ss >> loadedLight.color.r >> loadedLight.color.g >> loadedLight.color.b;
        } else if (key == "lightAmbient:") {
            ss >> loadedLight.ambient.r >> loadedLight.ambient.g >> loadedLight.ambient.b;
        }
        // Detección de nuevo bloque de objeto
        else if (trimmed.find("- object:") != std::string::npos) {
            finishCurrentObject();
            inObjectBlock = true;
        }
        // Atributos del objeto activo
        else if (inObjectBlock) {
            if (key == "id:") {
                ss >> currentObj.id;
            } else if (key == "name:") {
                std::string restOfLine;
                std::getline(ss, restOfLine);
                currentObj.name = trim(restOfLine);
            } else if (key == "model:") {
                std::string restOfLine;
                std::getline(ss, restOfLine);
                currentObj.modelPath = trim(restOfLine);
            } else if (key == "position:") {
                ss >> currentObj.position.x >> currentObj.position.y >> currentObj.position.z;
            } else if (key == "rotation:") {
                ss >> currentObj.rotation.x >> currentObj.rotation.y >> currentObj.rotation.z;
            } else if (key == "scale:") {
                ss >> currentObj.scale.x >> currentObj.scale.y >> currentObj.scale.z;
            } else if (key == "color:") {
                ss >> currentObj.color.r >> currentObj.color.g >> currentObj.color.b >> currentObj.color.a;
            } else if (key == "visible:") {
                int val = 1; ss >> val; currentObj.visible = (val != 0);
            } else if (key == "wireframe:") {
                int val = 0; ss >> val; currentObj.wireframe = (val != 0);
            } else if (key == "vertices:") {
                int val = 0; ss >> val; currentObj.vertices = (val != 0);
            } else if (key == "normals:") {
                int val = 0; ss >> val; currentObj.normals = (val != 0);
            } else if (key == "boundingbox:") {
                int val = 0; ss >> val; currentObj.bbox = (val != 0);
            }
        }
    }
    finishCurrentObject();
    in.close();

    // 4. Reconstrucción de la escena en memoria
    m_scene.clear();

    m_scene.getBackgroundColor() = loadedBgColor;
    m_scene.getLight() = loadedLight;

    // Caché de modelos en memoria para preservar el patrón Flyweight
    // Si varios objetos usan el mismo modelo .obj o primitiva, comparten el mismo puntero en GPU
    std::unordered_map<std::string, std::shared_ptr<Model>> modelCache;

    for (const auto& data : loadedObjects) {
        std::shared_ptr<Model> model = nullptr;

        // Comprobar si el modelo ya fue instanciado en esta carga
        auto it = modelCache.find(data.modelPath);
        if (it != modelCache.end()) {
            model = it->second;
        } else {
            // Caso A: Primitiva Procedimental matemática
            if (data.modelPath.find("[Procedural]") != std::string::npos) {
                if (data.modelPath.find("Piramide") != std::string::npos) {
                    model = Primitives::createPyramid(1.0f, 1.0f);
                } else if (data.modelPath.find("Esfera") != std::string::npos) {
                    model = Primitives::createSphere(1.0f);
                } else if (data.modelPath.find("Cilindro") != std::string::npos) {
                    model = Primitives::createCylinder(1.0f, 1.0f);
                } else {
                    model = Primitives::createCube(1.0f);
                }
            } 
            // Caso B: Modelo poligonal .obj de disco
            else {
                model = std::make_shared<Model>(data.modelPath);
                if (!model->isLoaded()) {
                    std::cerr << "[SceneSerializer WARN]: No se pudo cargar: " << data.modelPath 
                              << ". Reemplazando con cubo por defecto." << std::endl;
                    model = Primitives::createCube(1.0f);
                }
            }

            modelCache[data.modelPath] = model;
        }

        // Agregar la entidad a la escena y restaurar todas sus propiedades
        auto obj = m_scene.addObject(data.name, model);
        if (obj) {
            if (data.id > 0) {
                obj->id = data.id;
            }
            obj->transform.position = data.position;
            obj->transform.rotation = data.rotation;
            obj->transform.scale = data.scale;
            obj->color = data.color;
            obj->visible = data.visible;
            obj->showWireframe = data.wireframe;
            obj->showVertices = data.vertices;
            obj->showNormals = data.normals;
            obj->showBoundingBox = data.bbox;
        }
    }

    std::cout << "[SceneSerializer INFO]: Escena cargada y reconstruida exitosamente desde: " 
              << filepath << " (" << loadedObjects.size() << " objetos restaurados)" << std::endl;
    return true;
}
