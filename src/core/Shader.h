#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

/**
 * @brief Encapsula la carga, compilación, enlace y gestión de uniforms de un programa de shaders OpenGL.
 * 
 * Implementa el patrón RAII y semántica de movimiento para garantizar la liberación
 * correcta de los recursos en la GPU (glDeleteProgram).
 */
class Shader {
public:
    
    GLuint ID{0};
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void use() const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    GLint getUniformLocation(const std::string& name) const;
};
