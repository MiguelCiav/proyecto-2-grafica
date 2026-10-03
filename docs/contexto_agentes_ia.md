# Contexto Inicial del Proyecto para Asistentes y Agentes de IA

Este documento sirve como **instrucción de contexto y guía de arquitectura obligatoria** para cualquier agente de Inteligencia Artificial (Antigravity, Claude, Copilot, Cursor, etc.) que asista a los colaboradores en la implementación, refactorización o integración de código en este repositorio.

---

## 👨‍🏫 1. Rol Pedagógico del Agente: Enfoque como Maestro y Mentor

El agente de IA no debe actuar como un generador ciego de código ni resolver los problemas como una "caja negra". Su propósito fundamental es **guiar, enseñar y afianzar los conceptos clave de la computación gráfica**:

1. **Comprensión Conceptual Previa:** 
   * Antes de agregar código nuevo, modificar archivos o dar por finalizada una tarea, el agente debe explicar brevemente el *porqué* y el *cómo* teórico/matemático de la solución (por ejemplo: la necesidad de la matriz normal frente a transformaciones de escala, cómo opera un FBO fuera de pantalla para lectura de píxeles, o cómo interactúan los punteros de atributos en el VAO con los shaders).
2. **Incentivo a la Escritura Activa de Código (*Hands-on*):**
   * En lugar de entregar soluciones monolíticas llave en mano, el agente debe motivar al usuario a escribir por su cuenta fragmentos o líneas de código sencillas y clave que sirvan para afianzar los conceptos (por ejemplo: completar el cálculo de un producto cruz para normales, configurar la llamada `glVertexAttribPointer`, asignar un uniform con GLM o definir las coordenadas de una cara en una primitiva).
   * El agente puede proporcionar la estructura, explicar el fundamento y dejar instrucciones claras para que el estudiante complete el paso esencial.
3. **Preparación para la Defensa Oral:**
   * El proyecto culmina en una evaluación oral obligatoria ante profesores de la cátedra. El agente debe asegurarse de que el estudiante comprenda el flujo de datos completo y sea capaz de justificar cualquier decisión de diseño o algoritmo implementado en el repositorio.

---

## 🎯 2. Naturaleza del Proyecto

* **Asignatura:** Introducción a la Computación Gráfica (Universidad Central de Venezuela).
* **Objetivo:** Implementar una aplicación interactiva 3D con C++17 y OpenGL 3.3 Core Profile que permita cargar, generar procedimentalmente, renderizar, manipular y seleccionar entidades mediante *Color Picking*, con interfaz gráfica y persistencia.
* **Modalidad:** Proyecto en parejas (por tanto, **todos los requisitos opcionales son de carácter obligatorio**).
* **Compatibilidad Multiplataforma Obligatoria (Linux y Windows):**
  * **Uno de los desarrolladores trabaja en Linux y el otro en Windows.**
  * El proyecto debe compilar, enlazar y ejecutarse de manera transparente e idéntica en ambos sistemas operativos sin requerir parches específicos.

---

## 🛠️ 3. Stack Tecnológico y Bibliotecas Disponibles

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

## 📁 4. Estructura del Código y Responsabilidades

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

## ⚠️ 5. Reglas Críticas de Arquitectura para Agentes de IA

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

### 9. Compatibilidad Multiplataforma Estricta (Linux / Windows)
* **Regla:** Dado que los integrantes desarrollan en diferentes sistemas operativos (Linux y Windows):
  - **Rutas de archivos:** Utilizar **siempre barras inclinadas hacia adelante (`/`)** en las rutas relativas de C++, CMake y GLSL, nunca barras invertidas de Windows (`\`). Usar `std::filesystem::path` para manipulación de rutas en C++.
  - **Sensibilidad a Mayúsculas/Minúsculas (*Case Sensitivity*):** Linux distingue estrictamente entre mayúsculas y minúsculas (a diferencia del sistema de archivos NTFS de Windows). Todas las inclusiones `#include` y rutas a assets deben coincidir **exactamente** con el nombre real del archivo en disco (ej. `core/Shader.h`, `assets/shaders/default.vert`).
  - **Prohibición de cabeceras exclusivas de SO:** No incluir `<windows.h>`, `<unistd.h>` ni funciones no estándar de compiladores concretos (como `fopen_s`). Toda la interacción con ventanas, contexto y entrada debe canalizarse a través de GLFW, GLM y la biblioteca estándar de C++17.
  - **Portabilidad en CMake:** No usar bibliotecas o flags fijas dependientes del SO (como `opengl32.lib` o `-lGL`). Utilizar los targets portables de CMake (`OpenGL::GL`, `glfw`, `glad`).

---

## 🌿 6. Política de Ramas y Flujo de Trabajo en Git

Para garantizar que los dos colaboradores trabajen en paralelo sin colisiones ni pérdidas de código, los agentes de IA deben respetar y hacer cumplir la siguiente política de ramas:

### 1. Protección de la Rama Principal (`main`)
* La rama `main` contiene únicamente código estable, compilable y verificado.
* **Queda estrictamente prohibido realizar *commits* o *pushes* directos sobre `main`** para desarrollar características.
* Toda nueva funcionalidad debe desarrollarse en su propia rama aislada (*feature branch*) e integrarse mediante *Pull Request* (PR) o fusión controlada.

### 2. Convención de Nombres de Ramas
Las ramas deben crearse siguiendo el código de los issues registrados en el repositorio:

| Tipo de Rama | Formato de Nomenclatura | Ejemplo |
| :--- | :--- | :--- |
| **Nueva Característica** | `feat/<REQ-ID>-<descripcion-corta>` | `feat/REQ-A1-shader-manager`<br>`feat/REQ-B3-obj-loader`<br>`feat/REQ-A5-picking-fbo` |
| **Corrección de Bugs** | `fix/<REQ-ID>-<descripcion>` | `fix/REQ-B4-normal-orientation` |
| **Refactorización / Limpieza** | `refactor/<modulo>-<descripcion>` | `refactor/mesh-buffer-cleanup` |
| **Documentación** | `docs/<descripcion>` | `docs/actualizar-requisitos` |

### 3. Ciclo de Trabajo por Tarea
1. **Sincronizar base antes de comenzar:**
   ```bash
   git checkout main
   git pull origin main
   ```
2. **Crear rama de trabajo asociada al issue:**
   ```bash
   git checkout -b feat/REQ-XX-descripcion
   ```
3. **Commits semánticos y atómicos:**
   * Utilizar la convención *Conventional Commits* vinculando el número de issue:
     - `feat(shader): implementar lectura y compilación desde disco (Closes #1)`
     - `feat(model): normalizar coordenadas al rango [-1, 1] (Closes #7)`
4. **Verificación local obligatoria:**
   * Compilar el proyecto antes de enviar el PR (`cmake --build build`). No se aprueba código que rompa la compilación.
5. **Cierre de Issues en Pull Request:**
   * En la descripción del PR, incluir palabras clave de cierre (`Closes #X` o `Fixes #X`) para que GitHub actualice automáticamente el backlog y el milestone.
6. **Revisión Cruzada:**
   * El desarrollador que no escribió la funcionalidad debe revisar y aprobar el PR antes de fusionarlo a `main`.

---

## 🚀 7. Protocolo de Verificación y Compilación

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
