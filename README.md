# Proyecto Base OpenGL 3.3 Core con CMake

Plantilla base en **C++17** y **OpenGL 3.3 Core** configurada con **CMake 3.20+**, gestionando todas sus dependencias automáticamente y proporcionando una aplicación de ejemplo funcional con iluminación 3D, interfaz de usuario interactiva y carga de modelos Wavefront OBJ.

---

## 🛠 Tecnologías y Dependencias

| Dependencia | Versión / Estándar | Propósito | Integración |
| :--- | :--- | :--- | :--- |
| **C++ Standard** | **C++17** | Lenguaje y características modernas | Nativo en CMake |
| **CMake** | **3.20+** | Sistema de construcción multiplataforma | `CMakeLists.txt` |
| **OpenGL** | **3.3 Core** | API de gráficos 3D moderna basada en shaders | `find_package(OpenGL)` |
| **GLAD** | **3.3 Core** | Cargador de punteros de funciones OpenGL | Incluido en `external/glad` |
| **GLFW** | **3.4** | Creación de ventanas, contexto y entrada de usuario | `FetchContent` (Git) |
| **GLM** | **1.0.1** | Matemáticas (vectores, matrices, MVP) | `FetchContent` (Git) |
| **Dear ImGui** | **1.91.8** | Interfaz gráfica inmediata (UI de control en tiempo real) | `FetchContent` (Git) + backends GLFW/OpenGL3 |
| **tinyobjloader** | **v2.0.0rc13** | Carga y parseo de modelos 3D en formato `.obj` | `FetchContent` (Git) |

---

## 📁 Estructura del Proyecto

```text
proyecto-2-grafica/
├── CMakeLists.txt              # Configuración principal de CMake
├── README.md                   # Esta documentación
├── .gitignore                  # Exclusión de binarios y temporales
├── assets/                     # Recursos copiados automáticamente al compilar
│   ├── models/
│   │   └── cube.obj            # Modelo 3D de ejemplo (Cubo con normales y UVs)
│   └── shaders/
│       ├── basic.vert          # Vertex shader (transformación MVP y normales)
│       └── basic.frag          # Fragment shader (modelo de iluminación Phong)
├── external/
│   └── glad/                   # Cargador GLAD pre-generado para OpenGL 3.3 Core
│       ├── include/
│       │   ├── glad/glad.h
│       │   └── KHR/khrplatform.h
│       ├── src/glad.c
│       └── CMakeLists.txt
└── src/
    ├── main.cpp                # Punto de entrada y bucle de renderizado
    └── Shader.h                # Clase utilitaria para carga y compilación de shaders
```

---

## 🚀 Requisitos Previos

Antes de compilar, asegúrate de tener instalado:

1. **CMake** (versión 3.20 o superior):
   - Verificar en terminal: `cmake --version`
2. **Git** (necesario para que CMake descargue GLFW, GLM, ImGui y tinyobjloader automáticamente):
   - Verificar en terminal: `git --version`
3. **Compilador C++ compatible con C++17**:
   - **Windows**:
     - [Visual Studio Community](https://visualstudio.microsoft.com/vs/) con la carga de trabajo **"Desarrollo para el escritorio con C++"** (MSVC), o bien **Visual Studio Build Tools**.
     - Alternativa: **MinGW-w64** (con `g++` y `ninja` o `mingw32-make`).
   - **Linux (Ubuntu/Debian)**:
     ```bash
     sudo apt update
     sudo apt install -y build-essential cmake git libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libx11-dev
     ```
   - **macOS**:
     ```bash
     xcode-select --install
     ```

---

## 💻 Instrucciones de Compilación por Terminal

Abre tu terminal favorita (PowerShell, CMD, Git Bash, Bash o Zsh) en la raíz del proyecto:

### 1. Configurar y generar los archivos de construcción

Ejecuta el siguiente comando para crear la carpeta `build/` y resolver dependencias:

```bash
cmake -B build
```

> [!NOTE]
> Durante la primera ejecución de `cmake -B build`, CMake descargará automáticamente GLFW, GLM, ImGui y tinyobjloader mediante Git. Las compilaciones posteriores serán instantáneas.

#### Especificar un generador (opcional)
Si tienes varias herramientas instaladas o deseas usar un entorno específico:

- **Visual Studio 2022**:
  ```bash
  cmake -B build -G "Visual Studio 17 2022" -A x64
  ```
- **Ninja** (rápido y multiplataforma):
  ```bash
  cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
  ```
- **MinGW**:
  ```bash
  cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
  ```

---

### 2. Compilar el proyecto

Independientemente del generador o sistema operativo que uses, compila con:

```bash
cmake --build build --config Release
```

> [!TIP]
> Puedes acelerar la compilación usando todos los núcleos del CPU añadiendo `--parallel`:
> ```bash
> cmake --build build --config Release --parallel
> ```

---

### 3. Ejecutar la aplicación

Una vez terminada la compilación, el ejecutable estará en el directorio de salida:

#### En Windows (con Visual Studio / MSVC):
```powershell
.\build\Release\ProyectoGrafica.exe
```
*(Si compilaste en modo Debug, estará en `.\build\Debug\ProyectoGrafica.exe`)*.

#### En Windows (con MinGW o Ninja):
```powershell
.\build\ProyectoGrafica.exe
```

#### En Linux / macOS:
```bash
./build/ProyectoGrafica
```

---

## 🎮 Características del Ejemplo Incluido

Al ejecutar la aplicación verás:

1. **Ventana OpenGL 3.3 Core Profile** administrada por GLFW a 60+ FPS con V-Sync.
2. **Modelo 3D cargado con tinyobjloader**:
   - Carga el archivo `assets/models/cube.obj` parseando vértices y normales.
   - Cuenta con un cubo de respaldo embebido en caso de que el archivo no esté accesible.
3. **Matemáticas con GLM**:
   - Cálculo dinámico de matrices Modelo, Vista y Proyección (`MVP`).
   - Rotación libre en los 3 ejes (Pitch, Yaw, Roll) o rotación automática.
4. **Shaders GLSL 330 Core**:
   - `basic.vert` y `basic.frag` implementan el modelo de iluminación **Phong** (componente ambiental, difuso y especular).
5. **Panel Interactivo de Dear ImGui**:
   - **Transformaciones**: Activar/desactivar auto-rotación, ajustar velocidad y modificar ángulos manualmente.
   - **Iluminación y Material**: Selectores de color en tiempo real para el objeto, la luz, el fondo de la ventana y la posición de la fuente de luz.
   - **Renderizado**: Conmutador para modo *Wireframe* (`glPolygonMode`), visualización del recuento de vértices y triángulos.
   - **Diagnóstico**: Información de Vendor, Renderer y versiones de OpenGL / GLSL reportadas por el hardware, además de contador de FPS.

---

## 🧹 Limpiar el Proyecto

Para hacer una compilación totalmente limpia:

- **Windows (PowerShell)**:
  ```powershell
  Remove-Item -Recurse -Force build
  ```
- **Linux / macOS / Git Bash**:
  ```bash
  rm -rf build
  ```
