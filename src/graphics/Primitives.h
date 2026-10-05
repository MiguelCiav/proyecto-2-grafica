#pragma once

#include "graphics/Mesh.h"
#include "graphics/Model.h"
#include <memory>
#include <vector>

/**
 * @brief Generador procedimental de primitivas geométricas elementales con normales analíticas.
 * Implementación de los requisitos de REQ-08 / REQ-A3:
 *  - Cubo parametrizable en tamaño de arista.
 *  - Pirámide parametrizable en base y altura.
 *  - Esfera parametrizable en radio, sectores y anillos (stacks).
 *  - Cilindro parametrizable en radio, altura y sectores de revolución (Requisito Parejas).
 */
namespace Primitives {

    // --- Generación de Mesh puro ---

    /**
     * @brief Genera una malla de cubo centrada en el origen con normales analíticas por cara.
     * @param size Longitud de la arista del cubo (por defecto 1.0f).
     */
    Mesh createCubeMesh(float size = 1.0f);

    /**
     * @brief Genera una malla de pirámide de base cuadrada centrada en el origen con normales analíticas.
     * @param base Ancho y profundidad de la base cuadrada (por defecto 1.0f).
     * @param height Altura total desde la base al ápice (por defecto 1.0f).
     */
    Mesh createPyramidMesh(float base = 1.0f, float height = 1.0f);

    /**
     * @brief Genera una malla de esfera UV centrada en el origen con normales radiales analíticas.
     * @param radius Radio de la esfera (por defecto 1.0f).
     * @param sectors Número de subdivisiones longitudinales (mínimo 3, por defecto 32).
     * @param stacks Número de subdivisiones latitudinales (mínimo 2, por defecto 16).
     */
    Mesh createSphereMesh(float radius = 1.0f, unsigned int sectors = 32, unsigned int stacks = 16);

    /**
     * @brief Genera una malla de cilindro cerrada centrada en el origen con normales analíticas.
     * @param radius Radio de las tapas superior e inferior (por defecto 1.0f).
     * @param height Altura total del cilindro (por defecto 1.0f).
     * @param sectors Número de segmentos de revolución (mínimo 3, por defecto 32).
     */
    Mesh createCylinderMesh(float radius = 1.0f, float height = 1.0f, unsigned int sectors = 32);


    // --- Generación de Model (listo para SceneObject / Scene) ---

    std::shared_ptr<Model> createCube(float size = 1.0f);
    std::shared_ptr<Model> createPyramid(float base = 1.0f, float height = 1.0f);
    std::shared_ptr<Model> createSphere(float radius = 1.0f, unsigned int sectors = 32, unsigned int stacks = 16);
    std::shared_ptr<Model> createCylinder(float radius = 1.0f, float height = 1.0f, unsigned int sectors = 32);

}
