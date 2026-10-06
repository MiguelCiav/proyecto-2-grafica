#include "core/Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (const std::ifstream::failure& e) {
        std::cerr << "[ERROR::SHADER] No se pudo leer el archivo de shader: "
                  << vertexPath << " o " << fragmentPath
                  << "\nDetalle: " << e.what() << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertex = 0;
    GLuint fragment = 0;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
}

Shader::Shader(Shader&& other) noexcept : ID(other.ID) {
    other.ID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (ID != 0) {
            glDeleteProgram(ID);
        }
        ID = other.ID;
        other.ID = 0;
    }
    return *this;
}

void Shader::use() const {
    glUseProgram(ID);
}

GLint Shader::getUniformLocation(const std::string& name) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    return location;
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec2(const std::string& name, float x, float y) const {
    glUniform2f(getUniformLocation(name), x, y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(getUniformLocation(name), x, y, z, w);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}
