cmake_minimum_required(VERSION 3.20)
project(ProyectoGrafica LANGUAGES C CXX)

# Estándar de C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generar compile_commands.json (útil para editores como VS Code / clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configuración de OpenGL
find_package(OpenGL REQUIRED)

# GLAD (Cargador OpenGL 3.3 Core en external/glad)
add_subdirectory(external/glad)

# Gestión de dependencias automáticas mediante FetchContent
include(FetchContent)

# 1. GLFW 3
message(STATUS "Configurando GLFW 3...")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 2. GLM (OpenGL Mathematics)
message(STATUS "Configurando GLM...")
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

if(TARGET glm::glm)
    set(GLM_LIB glm::glm)
else()
    set(GLM_LIB glm)
endif()

# 3. tinyobjloader
message(STATUS "Configurando tinyobjloader...")
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG v2.0.0rc13
    GIT_SHALLOW TRUE
)
set(TINYOBJLOADER_BUILD_TEST_LOADER OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tinyobjloader)

if(TARGET tinyobjloader::tinyobjloader)
    set(TINYOBJ_LIB tinyobjloader::tinyobjloader)
else()
    set(TINYOBJ_LIB tinyobjloader)
endif()

# 4. Dear ImGui (con backend GLFW + OpenGL3)
message(STATUS "Configurando Dear ImGui...")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw glad OpenGL::GL)
endif()

# Fuentes y cabeceras del proyecto
set(SOURCES
    src/core/Shader.cpp
    src/core/Camera.cpp
    src/graphics/Mesh.cpp
    src/graphics/Model.cpp
    src/graphics/Primitives.cpp
    src/graphics/Framebuffer.cpp
    src/scene/Scene.cpp
    src/scene/SceneSerializer.cpp
    src/ui/EditorUI.cpp
    src/main.cpp
)

set(HEADERS
    src/core/Shader.h
    src/core/Camera.h
    src/graphics/Mesh.h
    src/graphics/Model.h
    src/graphics/Primitives.h
    src/graphics/Framebuffer.h
    src/scene/Scene.h
    src/scene/SceneSerializer.h
    src/ui/EditorUI.h
)

# Ejecutable principal
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    ${GLM_LIB}
    ${TINYOBJ_LIB}
    imgui
    OpenGL::GL
)

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++ -static)
endif()

# Copiar carpeta assets automáticamente junto al ejecutable generado
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copiando carpeta assets al directorio de salida..."
)
