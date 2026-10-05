#pragma once

#include <string>

class Scene;

/**
 * @brief Serializa y deserializa el estado completo de la escena 3D a/desde disco.
 * Almacena entidades, geometrías (.obj y primitivas procedimentales), transformaciones,
 * materiales difusos, canal alfa, banderas de visualización, iluminación direccional
 * y color de fondo de la ventana (REQ-10 / REQ-B7).
 */
class SceneSerializer {
public:
    explicit SceneSerializer(Scene& scene);
    ~SceneSerializer() = default;

    /**
     * @brief Guarda el estado completo de la escena en un archivo de texto estructurado (.scene).
     * @param filepath Ruta de destino en disco.
     * @return true si se guardó con éxito, false en caso contrario.
     */
    bool serialize(const std::string& filepath);

    /**
     * @brief Carga un archivo de escena (.scene), limpia la escena actual y restaura todas las entidades.
     * @param filepath Ruta del archivo a cargar.
     * @return true si se cargó y reconstruyó exitosamente, false en caso de error.
     */
    bool deserialize(const std::string& filepath);

    /**
     * @brief Obtiene el último mensaje descriptivo de error ocurrido durante la operación.
     */
    const std::string& getLastError() const { return m_lastError; }

private:
    Scene& m_scene;
    std::string m_lastError;
};
