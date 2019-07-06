#ifndef GLHELPERS_H
#define GLHELPERS_H
#include <QOpenGLFunctions_3_3_Core>
#include "skia/skiaincludes.h"

typedef QOpenGLFunctions_3_3_Core QGL33c;
#define BUFFER_OFFSET(i) ((void*)(i))

extern void checkGlErrors(const std::string& msg = "");

//! @brief Creates a program, compiles, and attaches associated shaders.
extern void iniProgram(QGL33c * const gl,
                       GLuint& program,
                       const QString &vShaderPath,
                       const QString &fShaderPath);

extern void iniTexturedVShaderVBO(QGL33c * const gl);
extern void iniTexturedVShaderVAO(QGL33c * const gl, GLuint& VAO);
extern void iniPlainVShaderVBO(QGL33c * const gl);
extern void iniPlainVShaderVAO(QGL33c * const gl, GLuint& VAO);

extern QString GL_PLAIN_VERT;
extern GLuint GL_PLAIN_SQUARE_VBO;

extern QString GL_TEXTURED_VERT;
extern GLuint GL_TEXTURED_SQUARE_VBO;

struct Texture {
    GLuint fId = 0;
    int fWidth = 0;
    int fHeight = 0;

    static Texture sCreateTextureFromImage(QGL33c * const gl,
                                           const std::string& imagePath);

    void bind(QGL33c * const gl) const;

    void clear(QGL33c * const gl);

    //! @brief Generates and binds texture.
    void gen(QGL33c * const gl);

    //! @brief Generates, binds texture and sets data.
    void gen(QGL33c * const gl,
             const int fWidth, const int fHeight,
             const void * const data);

    void swap(Texture& otherTexture) {
        std::swap(fId, otherTexture.fId);
        std::swap(fWidth, otherTexture.fWidth);
        std::swap(fHeight, otherTexture.fHeight);
    }

    sk_sp<SkImage> toImage(QGL33c * const gl) const;
private:
    bool loadImage(QGL33c * const gl, const std::string& imagePath);
};

struct TextureFrameBuffer {
    Texture fTexture;
    GLuint fFBOId = 0;
    int fWidth = 0;
    int fHeight = 0;
    bool fBound = false;

    //! @brief Swaps underlying texture and bind FBO
    void swapTexture(QGL33c * const gl, Texture& otherTexture) {
        fTexture.swap(otherTexture);

        bind(gl);
        // create a color attachment texture
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, fTexture.fId, 0);
    }

    void clear(QGL33c * const gl);

    void bind(QGL33c * const gl);
    void unbind() { fBound = false; }

    void bindTexture(QGL33c * const gl);

    //! @brief Generates and binds framebuffer and associated texture.
    void gen(QGL33c * const gl, const int width, const int height);

    sk_sp<SkImage> toImage(QGL33c * const gl) const {
        return fTexture.toImage(gl); }
};

#include <QJSEngine>
struct GpuRenderData {
    //! @brief Position in scene coordinates
    GLfloat fPosX;
    GLfloat fPosY;

    //! @brief Texture size
    GLuint fWidth;
    GLuint fHeight;

    //! @brief Used for shader based effects
    QJSEngine fJSEngine;
};

class GpuRenderTools {
public:
    GpuRenderTools(QGL33c* const gl, const sk_sp<SkImage>& img,
                   const GLuint texturedSquareVAO) :
        mGL(gl), mSquareVAO(texturedSquareVAO) {
        SkPixmap pix;
        img->peekPixels(&pix);
        mSrcTexture.gen(gl, img->width(), img->height(), pix.addr());
    }

    ~GpuRenderTools() {
        mTargetTextureFbo.clear(mGL);
        mSrcTexture.clear(mGL);
    }

    GLuint getSquareVAO() const { return mSquareVAO; }

    //! @brief Swaps the source and the target texture if.
    void swapTextures() {
        if(!validTargetFbo()) return;
        mTargetTextureFbo.swapTexture(mGL, mSrcTexture);
    }

    //! @brief Returns SkCanvas associated with the target texture.
    //! If there is no SkCanvas new SkCanvas is created.
    SkCanvas* requestTargetCanvas() {
        if(!mCanvas) {
            requestTargetFbo();
            mGrContext = GrContext::MakeGL();
    //        GrGLTextureInfo texInfo;
    //        texInfo.fFormat = GR_GL_BGRA8;
    //        texInfo.fTarget = GR_GL_TEXTURE_2D;
    //        const auto grTex = GrBackendTexture(mTexture.fWidth, mTexture.fHeight,
    //                                            GrMipMapped::kNo, texInfo);
    //        mSurface = SkSurface::MakeFromBackendTexture(
    //                    mGrContext.get(),
    //                    grTex, kTopLeft_GrSurfaceOrigin, 0,
    //                    kBGRA_8888_SkColorType, nullptr, nullptr);
            GrGLFramebufferInfo fboInfo;
            fboInfo.fFBOID = mTargetTextureFbo.fFBOId;
            fboInfo.fFormat = GR_GL_BGRA8;
            const auto grFbo = GrBackendRenderTarget(mTargetTextureFbo.fWidth,
                                                     mTargetTextureFbo.fHeight,
                                                     0, 0, fboInfo);
            mCanvas = mSurface->getCanvas();
        }
        return mCanvas;
    }

    //! @brief Returned texture may be used as one wishes,
    //!  but has to be valid.
    Texture& getSrcTexture() { return mSrcTexture; }

    //! @brief Returns TextureFrameBuffer associated with the target texture.
    //! If there is no SkCanvas new SkCanvas is created.
    TextureFrameBuffer& requestTargetFbo() {
        if(mTargetTextureFbo.fFBOId == 0)
            mTargetTextureFbo.gen(mGL, mSrcTexture.fWidth, mSrcTexture.fHeight);
        return mTargetTextureFbo;
    }

    bool validTargetCanvas() const { return mCanvas; }
    bool validTargetFbo() const { return mTargetTextureFbo.fFBOId; }
private:
    QGL33c* const mGL;
    const GLuint mSquareVAO;

    Texture mSrcTexture;
    TextureFrameBuffer mTargetTextureFbo;

    sk_sp<GrContext> mGrContext;
    sk_sp<SkSurface> mSurface;
    SkCanvas* mCanvas = nullptr;
};

#endif // GLHELPERS_H
