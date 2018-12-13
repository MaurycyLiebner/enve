#ifndef GLHELPERS_H
#define GLHELPERS_H
#include <QOpenGLFunctions_3_3_Core>
typedef QOpenGLFunctions_3_3_Core QGL33c;
#include "skiaincludes.h"

extern void assertNoGlErrors();

//! @brief Creates a program, compiles, and attaches associated shaders.
extern void iniProgram(QGL33c* gl,
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

extern BlurProgram GL_BLUR_PROGRAM;

struct Texture {
    GLuint id;
    int width;
    int height;

    static Texture createTextureFromImage(QGL33c* gl,
                                          const std::string& imagePath);

    void bind(QGL33c* gl);

    void deleteTexture(QGL33c* gl);

    //! @brief Generates and binds texture.
    void gen(QGL33c* gl);

    //! @brief Generates, binds texture and sets data.
    void gen(QGL33c* gl,
             const int& width, const int& height,
             const void * const data);
private:
    void loadImage(QGL33c* gl, const std::string& imagePath);
};

struct TextureFrameBuffer {
    GLuint textureId;
    GLuint frameBufferId;
    int width;
    int height;

    sk_sp<SkImage> toImage();

    void deleteTexture(QGL33c* gl);

    void deleteFrameBuffer(QGL33c* gl);

    void bindFrameBuffer(QGL33c* gl);

    void bindTexture(QGL33c* gl);

    //! @brief Generates and binds framebuffer and associated texture.
    void gen(QGL33c* gl, const int& widthT, const int& heightT);
};

#endif // GLHELPERS_H
