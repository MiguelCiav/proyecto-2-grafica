# Estructura del Proyecto y Guía de Compilación

## Estructura del Proyecto

```text
├── assets/
│   ├── models/                  <- Archivos .obj y .mtl descargados o de prueba
│   └── shaders/
│       ├── default.vert         <- Vertex Shader con modelo de iluminación
│       ├── default.frag         <- Fragment Shader Lambert + Alfa
│       ├── picking.vert         <- Shader para renderizar IDs únicos a color
│       ├── picking.frag         <- Fragment Shader que codifica identificadores en RGB
│       ├── debug.vert           <- Shader simple para Bounding Box, normales y vértices
│       └── debug.frag           <- Fragment Shader de color plano para depuración
├── external/
│   └── glad/                    <- Cargador GLAD pre-generado para OpenGL 3.3 Core
├── src/
│   ├── core/
│   │   ├── Shader.h / .cpp      <- Lectura, compilación y asignación de uniforms
│   │   └── Camera.h / .cpp      <- Control de vista (LookAt), proyección y movimiento WASD + Mouse
│   ├── graphics/
│   │   ├── Mesh.h / .cpp        <- Estructura de vértices, VBO/VAO/EBO, dibujo wireframe/puntos
│   │   ├── Model.h / .cpp       <- TinyObjLoader, cálculo de normales, centrado/normalización
│   │   ├── Primitives.h / .cpp  <- Generación de Cubo, Pirámide, Esfera y Cilindro
│   │   └── Framebuffer.h / .cpp <- FBO para Color Picking (selección global, local y triángulo)
│   ├── scene/
│   │   ├── Scene.h / .cpp       <- Lista de objetos, transformaciones, eliminación, luz global
│   │   └── SceneSerializer.h/.cpp <- Guardar y cargar la escena en disco (formato texto o JSON)
│   ├── ui/
│   │   └── EditorUI.h / .cpp    <- Paneles de ImGui (FPS, propiedades del objeto, modos)
│   └── main.cpp                 <- Inicialización, loop principal e integración de subsistemas
├── CMakeLists.txt               <- Archivo de configuración de construcción con CMake
└── README.md                    <- Documentación de la estructura y compilación
```

### Explicación de los Elementos

* **`assets/`**: Almacena los recursos estáticos del proyecto. CMake copia automáticamente esta carpeta junto al ejecutable final.
  * **`models/`**: Modelos 3D en formato `.obj` junto con sus archivos de materiales `.mtl`.
  * **`shaders/`**: Programas GLSL (OpenGL Shading Language) empleados en las distintas pasadas de renderizado:
    * `default.vert` / `default.frag`: Shaders principales para renderizado con iluminación Lambert y canal alfa.
    * `picking.vert` / `picking.frag`: Shaders para selección mediante color (Color Picking), codificando IDs de objetos o triángulos en colores RGB.
    * `debug.vert` / `debug.frag`: Shaders utilitarios para dibujar cajas delimitadoras (Bounding Boxes), normales y vértices.
* **`external/glad/`**: Código fuente y cabeceras de GLAD configurado para OpenGL 3.3 Core Profile.
* **`src/`**: Código fuente en C++ del motor/aplicación:
  * **`core/`**:
    * `Shader`: Lee, compila y gestiona programas de shaders en OpenGL, además de asignar valores uniformes (`mat4`, `vec3`, `float`, etc.).
    * `Camera`: Maneja la cámara en primera/tercera persona mediante matriz LookAt, proyección en perspectiva y controles interactivos (WASD + Mouse).
  * **`graphics/`**:
    * `Mesh`: Abstracción de buffers de OpenGL (VAO, VBO, EBO) y control de los modos de dibujo (sólido, líneas/wireframe, puntos).
    * `Model`: Carga mallas 3D usando `tinyobjloader`, genera normales si no están presentes y calcula el centrado y normalización del modelo en el espacio de coordenadas.
    * `Primitives`: Generador procedimental de geometrías básicas (Cubo, Pirámide, Esfera y Cilindro).
    * `Framebuffer`: Maneja Framebuffer Objects (FBO) fuera de pantalla para lectura de píxeles (`glReadPixels`) y soporte de selección interactiva.
  * **`scene/`**:
    * `Scene`: Mantiene la lista de objetos de la escena, sus matrices de transformación (traslación, rotación, escala), eliminación de elementos y parámetros de iluminación global.
    * `SceneSerializer`: Serializa y deserializa el estado de la escena en archivos de texto/JSON en disco.
  * **`ui/`**:
    * `EditorUI`: Integra Dear ImGui para desplegar paneles de control en tiempo real (contador de FPS, modos de dibujo, propiedades de los objetos e iluminación).
  * **`main.cpp`**: Punto de entrada del programa. Inicializa la ventana GLFW, el contexto OpenGL, el bucle principal y coordina todos los subsistemas.
* **`CMakeLists.txt`**: Script maestro de CMake que descarga dependencias mediante `FetchContent` (GLFW, GLM, ImGui, TinyObjLoader) y compila el ejecutable.

---

## Paso a Paso para Compilar (Terminal de VS Code)

Abre la terminal integrada en VS Code (`Ctrl + ~` o `Terminal -> New Terminal`) en la raíz del proyecto.

### En Linux

1. **Configurar el proyecto:**
   ```bash
   cmake -B build
   ```

2. **Compilar:**
   ```bash
   cmake --build build
   ```

3. **Ejecutar:**
   ```bash
   ./build/ProyectoGrafica
   ```

---

### En Windows

1. **Configurar el proyecto:**
   ```powershell
   cmake -B build
   ```

2. **Compilar:**
   ```powershell
   cmake --build build --config Release
   ```

3. **Ejecutar:**
   * Si compilas con **Visual Studio (MSVC)**:
     ```powershell
     .\build\Release\ProyectoGrafica.exe
     ```
   * Si compilas con **MinGW** o **Ninja**:
     ```powershell
     .\build\ProyectoGrafica.exe
     ```
