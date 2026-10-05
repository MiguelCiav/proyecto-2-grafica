# Estructura del Proyecto y Guía de Compilación

## Estructura del Proyecto

```text
├── assets/
│   ├── models/                  <- Archivos .obj y .mtl descargados o de prueba
│   ├── scenes/                  <- Archivos de persistencia de escenas 3D (.scene)
│   └── shaders/
│       ├── base.vert            <- Vertex Shader con modelo de iluminación
│       ├── base.frag            <- Fragment Shader Lambert + Alfa
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
  * **`scenes/`**: Archivos de escenas guardadas (`.scene`) que contienen la definición completa del entorno, entidades y materiales.
  * **`shaders/`**: Programas GLSL (OpenGL Shading Language) empleados en las distintas pasadas de renderizado:
    * `base.vert` / `base.frag`: Shaders principales para renderizado con iluminación Lambert y canal alfa.
    * `picking.vert` / `picking.frag`: Shaders para selección mediante color (Color Picking), codificando IDs de objetos o triángulos en colores RGB.
    * `debug.vert` / `debug.frag`: Shaders utilitarios para dibujar cajas delimitadoras (Bounding Boxes), normales y vértices.
* **`external/glad/`**: Código fuente y cabeceras de GLAD configurado para OpenGL 3.3 Core Profile.
* **`src/`**: Código fuente en C++ del motor/aplicación:
  * **`core/`**:
    * `Shader`: Lee los archivos `.vert` y `.frag` del disco, compila y gestiona el enlace, eliminando cadenas de texto en C++, además de asignar uniforms.
    * `Camera`: Calcula las matrices `view` y `projection` requeridas por el vertex shader provisto y procesa el mouse y teclado GLFW.
  * **`graphics/`**:
    * `Mesh`: Encapsula el VAO, VBO y EBO de un sub-mallado individual y gestiona los modos de dibujo (sólido, líneas/wireframe, puntos).
    * `Model`: Almacena los sub-mallados (mediante `tinyobjloader`), computa las normales promedio si faltan, escala la figura a $[-1, 1]$ (normalización de tamaño) y guarda el color difuso $K_d$ del `.mtl`.
    * `Primitives`: Genera matemáticamente las posiciones y normales para el cubo, pirámide, esfera y cilindro sin depender de archivos externos.
    * `Framebuffer`: Dibuja la escena en una textura oculta asignando un color RGB único por objeto, sub-mallado o triángulo para leer el píxel bajo el cursor (`glReadPixels`).
  * **`scene/`**:
    * `Scene`: Contenedor maestro que actualiza la jerarquía cuando se rota, traslada o escala un objeto y propaga los cambios a sus sub-mallados, además de administrar la luz global.
    * `SceneSerializer`: Guarda y carga la escena en disco (formato texto o JSON).
  * **`ui/`**:
    * `EditorUI`: Centraliza todas las llamadas de Dear ImGui para evitar ensuciar el ciclo de renderizado (FPS, propiedades del objeto, modos).
  * **`main.cpp`**: Inicialización, loop principal e integración de subsistemas.
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

---

## Generación del Paquete de Entrega (.zip)

Para generar el archivo comprimido final requerido para la evaluación (`PROY2_CEDULA1_CEDULA2.zip`), ejecute el script multiplataforma en la raíz del proyecto pasando las cédulas de ambos integrantes:

```bash
python3 scripts/package_submission.py <CEDULA_INTEGRANTE_1> <CEDULA_INTEGRANTE_2>
```

Ejemplo:
```bash
python3 scripts/package_submission.py 28123456 29654321
```

Este script empaqueta automáticamente el código fuente (`src/`, `external/`), recursos (`assets/`), configuración de CMake (`CMakeLists.txt`) y documentación (`README.md`), excluyendo de forma estricta directorios temporales, binarios de compilación (`build/`) o metadatos de Git (`.git/`).
