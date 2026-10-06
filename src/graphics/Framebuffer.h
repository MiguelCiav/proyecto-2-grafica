#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

/**
 * @brief Encapsula un Framebuffer Object (FBO) fuera de pantalla para Color Picking.
 * Dispone de una textura de color RGB (con filtro GL_NEAREST para evitar interpolaciones)
 * y un Renderbuffer de profundidad (GL_DEPTH24_STENCIL8) para respetar el Z-Buffer durante la selección.
 * Gestiona sus recursos en la GPU mediante RAII.
 */
class Framebuffer
{
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    Framebuffer(const Framebuffer &) = delete;
    Framebuffer &operator=(const Framebuffer &) = delete;

    Framebuffer(Framebuffer &&other) noexcept;
    Framebuffer &operator=(Framebuffer &&other) noexcept;

    void bind() const;
    void unbind() const;

    void rescale(int width, int height);

    unsigned int readPixel(int x, int y, bool flipY = true) const;
    unsigned int readPixelID(int x, int y, bool flipY = true) const { return readPixel(x, y, flipY); }

    glm::uvec4 readRawPixel(int x, int y, bool flipY = true) const;

    static glm::vec3 encodeID(unsigned int id);
    static unsigned int decodeID(unsigned char r, unsigned char g, unsigned char b);

    static unsigned int encodeLocalID(unsigned int objectId, unsigned int subMeshIndex);
    static void decodeLocalID(unsigned int id, unsigned int &outObjectId, int &outSubMeshIndex);

    unsigned int getFBO() const { return m_fbo; }
    unsigned int getTexture() const { return m_texture; }
    unsigned int getRBO() const { return m_rbo; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    bool isComplete() const { return m_complete; }

private:
    unsigned int m_fbo{0};
    unsigned int m_texture{0};
    unsigned int m_rbo{0};
    int m_width{0};
    int m_height{0};
    bool m_complete{false};

    void setupFramebuffer();
    void cleanup();
};
