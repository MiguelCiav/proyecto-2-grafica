#include "graphics/Framebuffer.h"
#include <iostream>
#include <algorithm>

Framebuffer::Framebuffer(int width, int height)
    : m_width(std::max(1, width)), m_height(std::max(1, height)) {
    setupFramebuffer();
}

Framebuffer::~Framebuffer() {
    cleanup();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : m_fbo(other.m_fbo),
      m_texture(other.m_texture),
      m_rbo(other.m_rbo),
      m_width(other.m_width),
      m_height(other.m_height),
      m_complete(other.m_complete) {
    other.m_fbo = 0;
    other.m_texture = 0;
    other.m_rbo = 0;
    other.m_width = 0;
    other.m_height = 0;
    other.m_complete = false;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        cleanup();

        m_fbo = other.m_fbo;
        m_texture = other.m_texture;
        m_rbo = other.m_rbo;
        m_width = other.m_width;
        m_height = other.m_height;
        m_complete = other.m_complete;

        other.m_fbo = 0;
        other.m_texture = 0;
        other.m_rbo = 0;
        other.m_width = 0;
        other.m_height = 0;
        other.m_complete = false;
    }
    return *this;
}

void Framebuffer::setupFramebuffer() {
    // 1. Crear y enlazar el Framebuffer Object (FBO)
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // 2. Crear la textura de color adjunta (Color Attachment)
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Filtros GL_NEAREST: Cruciales para Color Picking. Evitan que se promedien colores
    // entre polígonos adyacentes, lo cual generaría IDs espurios o inexistentes.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Conectar textura como Color Attachment 0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    // 3. Crear el Renderbuffer de profundidad y stencil (RBO)
    // Necesario para que el Z-Buffer opere normalmente durante la pasada de picking
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);

    // Conectar RBO como Depth/Stencil Attachment
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    // 4. Verificar integridad y completitud del Framebuffer
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        m_complete = true;
    } else {
        std::cerr << "[ERROR::FRAMEBUFFER]: Framebuffer no esta completo!" << std::endl;
        m_complete = false;
    }

    // Desenlazar para evitar escrituras accidentales
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::cleanup() {
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    if (m_rbo) {
        glDeleteRenderbuffers(1, &m_rbo);
        m_rbo = 0;
    }
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    m_complete = false;
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::rescale(int width, int height) {
    width = std::max(1, width);
    height = std::max(1, height);

    if (m_width == width && m_height == height) {
        return;
    }

    m_width = width;
    m_height = height;

    cleanup();
    setupFramebuffer();
}

unsigned int Framebuffer::readPixel(int x, int y, bool flipY) const {
    glm::uvec4 raw = readRawPixel(x, y, flipY);
    return decodeID(static_cast<unsigned char>(raw.r),
                    static_cast<unsigned char>(raw.g),
                    static_cast<unsigned char>(raw.b));
}

glm::uvec4 Framebuffer::readRawPixel(int x, int y, bool flipY) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height || !m_complete) {
        return glm::uvec4(0);
    }

    // En GLFW el origen (0,0) está arriba a la izquierda con Y hacia abajo.
    // En OpenGL el origen (0,0) está abajo a la izquierda con Y hacia arriba.
    int readY = flipY ? (m_height - 1 - y) : y;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    unsigned char pixel[4] = {0, 0, 0, 0};
    glReadPixels(x, readY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return glm::uvec4(pixel[0], pixel[1], pixel[2], pixel[3]);
}

glm::vec3 Framebuffer::encodeID(unsigned int id) {
    // Descomponer el entero en 3 componentes de 8 bits cada una (R, G, B)
    // y normalizar al rango [0.0, 1.0] para su emisión en el fragment shader
    float r = static_cast<float>(id & 0x000000FF) / 255.0f;
    float g = static_cast<float>((id & 0x0000FF00) >> 8) / 255.0f;
    float b = static_cast<float>((id & 0x00FF0000) >> 16) / 255.0f;
    return glm::vec3(r, g, b);
}

unsigned int Framebuffer::decodeID(unsigned char r, unsigned char g, unsigned char b) {
    // Reconstruir el ID entero original a partir de los bytes recibidos de glReadPixels
    return static_cast<unsigned int>(r) |
          (static_cast<unsigned int>(g) << 8) |
          (static_cast<unsigned int>(b) << 16);
}

unsigned int Framebuffer::encodeLocalID(unsigned int objectId, unsigned int subMeshIndex) {
    // 16 bits superiores: objectId (rango 1 a 65535)
    // 8 bits inferiores: subMeshIndex + 1 (rango 1 a 255)
    return ((objectId & 0xFFFF) << 8) | ((subMeshIndex + 1) & 0xFF);
}

void Framebuffer::decodeLocalID(unsigned int id, unsigned int& outObjectId, int& outSubMeshIndex) {
    outObjectId = (id >> 8) & 0xFFFF;
    unsigned int sub = id & 0xFF;
    outSubMeshIndex = (sub > 0) ? (static_cast<int>(sub) - 1) : -1;
}
