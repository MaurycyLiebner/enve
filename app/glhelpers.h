#ifndef GLHELPERS_H
#define GLHELPERS_H
#include <QOpenGLFunctions_3_3_Core>
typedef QOpenGLFunctions_3_3_Core QGL33c;
#include "skiaincludes.h"

extern void assertNoGlErrors();

//! @brief Creates a program, compiles, and attaches associated shaders.
extern bool iniProgram(QGL33c* gl,
                       GLuint& program,
                       const std::string& vShaderPath,
                       const std::string& fShaderPath);

extern void iniTexturedVShaderVBO(QGL33c* gl);
extern void iniTexturedVShaderVAO(QGL33c* gl, GLuint& VAO);
extern void iniPlainVShaderVBO(QGL33c* gl);
extern void iniPlainVShaderVAO(QGL33c* gl, GLuint& VAO);

extern std::string GL_PLAIN_VERT;
extern GLuint GL_PLAIN_SQUARE_VBO;

extern std::string GL_TEXTURED_VERT;
extern GLuint GL_TEXTURED_SQUARE_VBO;

struct ShaderProgram {
    GLuint fID;
};

struct BlurProgram : public ShaderProgram {
    GLint fBlurRadiusLoc;
};

struct DotProgram : public ShaderProgram {
    GLint fDotRadiusLoc;
    GLint fDotDistanceLoc;
    GLint fOnePixelLoc;
    GLint fTranslateLoc;
};

extern BlurProgram GL_BLUR_PROGRAM;
extern DotProgram GL_DOT_PROGRAM;

struct Texture {
    GLuint fID;
    int fWidth;
    int fHeight;

    static Texture createTextureFromImage(QGL33c* gl,
                                          const std::string& imagePath);

    void bind(QGL33c* gl);

    void deleteTexture(QGL33c* gl);

    //! @brief Generates and binds texture.
    void gen(QGL33c* gl);

    //! @brief Generates, binds texture and sets data.
    void gen(QGL33c* gl,
             const int& fWidth, const int& fHeight,
             const void * const data);

    void swap(Texture& otherTexture) {
        GLuint id = fID;
        int width = fWidth;
        int height = fHeight;

        fID = otherTexture.fID;
        fWidth = otherTexture.fWidth;
        fHeight = otherTexture.fHeight;

        otherTexture.fID = id;
        otherTexture.fWidth = width;
        otherTexture.fHeight = height;
    }
private:
    bool loadImage(QGL33c* gl, const std::string& imagePath);
};

struct TextureFrameBuffer {
    Texture fTexture;
    GLuint fFrameBufferId;
    int fWidth;
    int fHeight;

    void swapTexture(QGL33c* gl, Texture& otherTexture) {
        fTexture.swap(otherTexture);

        gl->glBindFramebuffer(GL_FRAMEBUFFER, fFrameBufferId);
        // create a color attachment texture
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, fTexture.fID, 0);
    }

    sk_sp<SkImage> toImage();

    void deleteTexture(QGL33c* gl);

    void deleteFrameBuffer(QGL33c* gl);

    void bindFrameBuffer(QGL33c* gl);

    void bindTexture(QGL33c* gl);

    //! @brief Generates and binds framebuffer and associated texture.
    void gen(QGL33c* gl, const int& widthT, const int& heightT);
};

#endif // GLHELPERS_H
