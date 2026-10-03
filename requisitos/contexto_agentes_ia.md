# Contexto Inicial del Proyecto para Asistentes y Agentes de IA

Este documento sirve como **instrucción de contexto y guía de arquitectura obligatoria** para cualquier agente de Inteligencia Artificial (Antigravity, Claude, Copilot, Cursor, etc.) que asista a los colaboradores en la implementación, refactorización o integración de código en este repositorio.

---

## 🎯 1. Naturaleza del Proyecto

* **Asignatura:** Introducción a la Computación Gráfica (Universidad Central de Venezuela).
* **Objetivo:** Implementar una aplicación interactiva 3D con C++17 y OpenGL 3.3 Core Profile que permita cargar, generar procedimentalmente, renderizar, manipular y seleccionar entidades mediante *Color Picking*, con interfaz gráfica y persistencia.
* **Modalidad:** Proyecto en parejas (por tanto, **todos los requisitos opcionales son de carácter obligatorio**).

---

## 🛠️ 2. Stack Tecnológico y Bibliotecas Disponibles

El proyecto ya tiene configuradas sus dependencias a través de `CMakeLists.txt`. Los agentes **no deben añadir paquetes externos manuales ni gestores adicionales**.

| Componente | Versión | Integración / Ubicación | Propósito |
| :--- | :--- | :--- | :--- |
| **C++ Standard** | **C++17** | Nativo en CMake | Lenguaje base |
| **CMake** | **3.20+** | Raíz `CMakeLists.txt` | Sistema de compilación multiplataforma |
| **OpenGL** | **3.3 Core** | `find_package(OpenGL)` | API gráfica moderna basada en shaders |
| **GLAD** | **3.3 Core** | `external/glad` | Cargador de punteros a funciones de OpenGL |
| **GLFW** | **3.4** | `FetchContent` (Git) | Ventanas, contexto y entrada de usuario |
| **GLM** | **1.0.1** | `FetchContent` (Git) | Álgebra lineal (vectores, matrices, transformaciones) |
| **Dear ImGui** | **1.91.8** | `FetchContent` (Git) | Interfaz gráfica inmediata (UI en tiempo real) |
| **tinyobjloader** | **v2.0.0rc13**| `FetchContent` (Git) | Parseo de geometrías `.obj` y materiales `.mtl` |

---

## 📁 3. Estructura del Código y Responsabilidades

Toda contribución de código debe ubicarse estrictamente dentro del módulo asignado:

```text
├── assets/
│   ├── models/                  <- Archivos .obj y .mtl descargados o de prueba
│   └── shaders/
│       ├── default.vert         <- Vertex Shader con modelo de iluminación (provisto por la cátedra)
│       ├── default.frag         <- Fragment Shader Lambert + canal Alfa (provisto por la cátedra)
│       ├── picking.vert         <- Shader para renderizado de IDs unívocos en color
│       ├── picking.frag         <- Emisión del ID codificado en formato RGB
│       ├── debug.vert           <- Shader auxiliar para líneas y puntos de depuración
│       └── debug.frag           <- Shader de color plano para depuración visual
├── external/
│   └── glad/                    <- Cargador GLAD pre-generado (no modificar)
├── src/
│   ├── core/
│   │   ├── Shader.h / .cpp      <- Lectura de archivos, compilación y paso de uniforms (sin strings GLSL en C++)
│   │   └── Camera.h / .cpp      <- Cámara LookAt, proyección en perspectiva y controles WASD + Mouse
│   ├── graphics/
│   │   ├── Mesh.h / .cpp        <- Encapsulación de VAO/VBO/EBO de un sub-mallado y modos de dibujo
│   │   ├── Model.h / .cpp       <- TinyObjLoader, normales promedio, normalización a [-1, 1] y color difuso Kd
│   │   ├── Primitives.h / .cpp  <- Generador procedimental de Cubo, Pirámide, Esfera y Cilindro
│   │   └── Framebuffer.h / .cpp <- Búfer FBO fuera de pantalla para lectura de píxeles (glReadPixels)
│   ├── scene/
│   │   ├── Scene.h / .cpp       <- Grafo/Lista de objetos, transformaciones, eliminación y luz global
│   │   └── SceneSerializer.h/.cpp <- Persistencia de la escena en disco (formato texto o JSON)
│   ├── ui/
│   │   └── EditorUI.h / .cpp    <- Paneles de ImGui (FPS, propiedades del objeto, modos, culling, depth test)
│   └── main.cpp                 <- Inicialización GLFW/GLAD, bucle principal y orquestación
├── CMakeLists.txt
└── README.md
```

---

## ⚠️ 4. Reglas Críticas de Arquitectura para Agentes de IA

Cualquier agente que genere o modifique código debe acatar sin excepción las siguientes reglas:

### 1. Inclusión Estricta de GLAD antes de GLFW
* **Regla:** `<glad/glad.h>` **siempre** debe incluirse antes que `<GLFW/glfw3.h>` o cualquier cabecera del sistema OpenGL.
* **Cabeceras C++ (`.h`):** Evitar incluir `<GLFW/glfw3.h>` en archivos `.h`. Usar *forward declaration* (`struct GLFWwindow;`) para no contaminar el orden de inclusión en unidades de traducción dependientes.

### 2. Integridad de los Shaders de la Cátedra
* **Regla:** El modelo de iluminación difusa (Lambert) en `assets/shaders/default.vert` y `default.frag` es suministrado por la cátedra. **Está estrictamente prohibido alterar la matemática de iluminación**.
* El código C++ debe suministrar los atributos (`aPos`, `aNormal`, `aTexCoords`) y uniforms requeridos (`model`, `view`, `projection`, `lightPos`, `lightColor`, `objectColor`, `alpha`).

### 3. Prohibición de Shaders Embebidos como Cadenas en C++
* **Regla:** Ningún shader debe declararse como `const char*` o `std::string` dentro de archivos C++. Todos deben residir en `assets/shaders/` y ser leídos desde el sistema de archivos por `Shader.h/.cpp`.

### 4. Modelo vs. Sub-mallado (`Model` vs `Mesh`)
* **Regla:** Un `Model` no es un único buffer; es un contenedor de uno o múltiples `Mesh` (sub-mallados).
* Cualquier transformación (traslación, rotación, escala) realizada sobre un objeto en modo **Global** debe propagarse a todos sus sub-mallados.
* La rotación debe calcularse respecto al **centro o eje propio del objeto**, no respecto al origen del mundo $(0,0,0)$.

### 5. Normalización y Materiales de Modelos OBJ
* Al cargar cualquier archivo `.obj`:
  - Si faltan las normales, calcularlas promediando las normales de cada triángulo adyacente a cada vértice.
  - Centrar el modelo en $(0,0,0)$ y escalarlo proporcionalmente para que quede acotado en el intervalo $[-1, 1]$.
  - Del archivo de materiales `.mtl` solo se debe extraer y emplear el color difuso ($K_d$).

### 6. Sistema de Selección por Color Picking (Obligatorio con FBO)
* **Regla:** La selección interactiva **no debe implementarse mediante Raycasting**. Debe utilizarse la técnica de **Color Picking por Framebuffer (FBO)**:
  1. Renderizar la escena en un FBO oculto utilizando `picking.vert` y `picking.frag`.
  2. Cada entidad se dibuja con un color RGB único que codifica su ID entero:
     $$\text{ID} \longleftrightarrow (R, G, B)$$
  3. Leer el píxel bajo el cursor usando `glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel)`.
  4. Soportar los tres niveles de selección: **Global** (objeto completo), **Local** (sub-mallado individual) y **Triángulo** (resaltando visualmente el triángulo seleccionado).

### 7. Gestión de Memoria en GPU (OpenGL RAII)
* Toda clase que genere identificadores de OpenGL (`glGenBuffers`, `glGenVertexArrays`, `glGenFramebuffers`, `glCreateProgram`, `glGenTextures`) debe encargarse de su liberación explícita (`glDelete*`) en su destructor o método `cleanup()`.
* Implementar o eliminar apropiadamente los constructores de copia y movimiento para evitar dobles liberaciones accidentales de recursos en GPU.

### 8. Desacoplamiento de la Interfaz (`EditorUI`)
* Mantener las llamadas de Dear ImGui confinadas dentro de `ui/EditorUI.cpp`. El ciclo principal en `src/main.cpp` debe permanecer limpio y comprensible.

---

## 🚀 5. Protocolo de Verificación y Compilación

Antes de dar por finalizada cualquier tarea o cambio sugerido por un agente, se debe verificar que el proyecto compile sin errores ni advertencias mediante la terminal:

```bash
# Configuración inicial (si la carpeta build no existe)
cmake -B build

# Compilación limpia
cmake --build build

# En Linux:
./build/ProyectoGrafica

# En Windows (PowerShell):
.\build\Release\ProyectoGrafica.exe  # Si usa Visual Studio / MSVC
.\build\ProyectoGrafica.exe          # Si usa MinGW o Ninja
```
