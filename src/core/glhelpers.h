#ifndef GLHELPERS_H
#define GLHELPERS_H
#include <QOpenGLFunctions_3_3_Core>
#include "skia/skiaincludes.h"
#include "exceptions.h"

typedef QOpenGLFunctions_3_3_Core QGL33;
#define BUFFER_OFFSET(i) ((void*)(i))
#define checkGlErrors \
    { \
        const GLenum glError = glGetError(); \
        if(glError != GL_NO_ERROR) \
            RuntimeThrow("OpenGL error " + std::to_string(glError)); \
    }

//! @brief Creates a program, compiles, and attaches associated shaders.
extern void iniProgram(QGL33 * const gl,
                       GLuint& program,
                       const QString &vShaderPath,
                       const QString &fShaderPath);

extern void iniTexturedVShaderVBO(QGL33 * const gl);
extern void iniTexturedVShaderVAO(QGL33 * const gl, GLuint& VAO);
extern void iniPlainVShaderVBO(QGL33 * const gl);
extern void iniPlainVShaderVAO(QGL33 * const gl, GLuint& VAO);

extern QString GL_PLAIN_VERT;
extern GLuint GL_PLAIN_SQUARE_VBO;

extern QString GL_TEXTURED_VERT;
extern GLuint GL_TEXTURED_SQUARE_VBO;

struct Texture {
    GLuint fId = 0;
    int fWidth = 0;
    int fHeight = 0;

    void bind(QGL33 * const gl) const;

    void clear(QGL33 * const gl);

    //! @brief Generates and binds texture.
    void gen(QGL33 * const gl);

    //! @brief Generates, binds texture and sets data.
    void gen(QGL33 * const gl,
             const int width, const int height,
             const void * const data);

    void set(QGL33 * const gl,
             const int width, const int height,
             const void * const data);

    void swap(Texture& otherTexture) {
        std::swap(fId, otherTexture.fId);
        std::swap(fWidth, otherTexture.fWidth);
        std::swap(fHeight, otherTexture.fHeight);
    }

    SkBitmap bitmapSnapshot(QGL33 * const gl) const;
    sk_sp<SkImage> imageSnapshot(QGL33 * const gl) const;
};

struct TextureFrameBuffer {
    Texture fTexture;
    GLuint fFBOId = 0;
    int fWidth = 0;
    int fHeight = 0;
    bool fBound = false;

    //! @brief Swaps underlying texture and bind FBO
    void swapTexture(QGL33 * const gl, Texture& otherTexture) {
        fTexture.swap(otherTexture);

        bind(gl);
        // create a color attachment texture
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, fTexture.fId, 0);
    }

    void clear(QGL33 * const gl);

    void bind(QGL33 * const gl);
    void unbind() { fBound = false; }

    void bindTexture(QGL33 * const gl);

    //! @brief Generates and binds framebuffer and associated texture.
    void gen(QGL33 * const gl, const int width, const int height);

    sk_sp<SkImage> toImage(QGL33 * const gl) const {
        return fTexture.imageSnapshot(gl);
    }

    SkBitmap toBitmap(QGL33 * const gl) const {
        return fTexture.bitmapSnapshot(gl);
    }
};

enum class HardwareSupport : short {
    CPU_ONLY,
    CPU_PREFFERED,
    GPU_PREFFERED,
    GPU_ONLY
};

struct CpuRenderData {
    //! @brief Pixel {0, 0} position in scene coordinates
    int fPosX;
    int fPosY;

    //! @brief Texture size
    uint fWidth;
    uint fHeight;
};

#include <QJSEngine>
struct GpuRenderData : public CpuRenderData {
    //! @brief Used for shader based effects
    QJSEngine fJSEngine;
};

class SwitchableContext {
    friend class GpuPostProcessor;
    enum class Mode {
        OpenGL,
        Skia
    };
public:
    //! @brief In Skia mode returns the handled GrContext,
    //! in OpenGL mode returns nullptr
    GrContext* requestContext() const {
        if(skiaMode()) return mContext.get();
        return nullptr;
    }

    bool skiaMode() const { return mMode == Mode::Skia; }
    bool openGLMode() const { return mMode == Mode::OpenGL; }

    void switchToSkia() { setMode(Mode::Skia); }
    void switchToOpenGL() { setMode(Mode::OpenGL); }

    GLuint textureSquareVAO() const { return mTexturedSquareVAO; }
private:
    void setContext(const sk_sp<GrContext>& context,
                    const GLuint textureSquareVAO) {
        mContext = context;
        mTexturedSquareVAO = textureSquareVAO;
    }

    void setMode(const Mode mode) {
        if(mode == mMode) return;
        mMode = mode;
        if(skiaMode()) mContext->resetContext();
    }

    sk_sp<GrContext> mContext;
    Mode mMode = Mode::Skia;
    GLuint mTexturedSquareVAO;
};
#include "skia/skiahelpers.h"
class CpuRenderTools {
public:
    CpuRenderTools(const SkBitmap& srcBtmp) : fSrcDst(srcBtmp) {}

    const SkBitmap fSrcDst;

    bool hasBackupBitmap() const { return !fBackupBtmp.isNull(); }

    SkBitmap requestBackupBitmap() {
        if(fBackupBtmp.isNull())
            fBackupBtmp.allocPixels(fSrcDst.info());
        return fBackupBtmp;
    }

    void swap() {
        if(fBackupBtmp.isNull()) return;
        fBackupBtmp.swap(*const_cast<SkBitmap*>(&fSrcDst));
    }
private:
    SkBitmap fBackupBtmp;
};

class GpuRenderTools {
public:
    GpuRenderTools(QGL33* const gl,
                   SwitchableContext& context,
                   const sk_sp<SkImage>& img) :
        mGL(gl), mContext(context) {
        if(img->isTextureBacked()) {
            const auto grTex = img->getBackendTexture(true);
            GrGLTextureInfo texInfo;
            grTex.getGLTextureInfo(&texInfo);
            mSrcTexture.fId = texInfo.fID;
            mSrcTexture.fWidth = grTex.width();
            mSrcTexture.fHeight = grTex.height();
        } else {
            SkPixmap pix;
            img->peekPixels(&pix);
            mSrcTexture.gen(gl, img->width(), img->height(), pix.addr());
        }
    }

    ~GpuRenderTools() {
        mTargetTextureFbo.clear(mGL);
        mSrcTexture.clear(mGL);
    }

    GLuint getSquareVAO() const { return mContext.textureSquareVAO(); }

    //! @brief Swaps the source and the target texture if valid.
    void swapTextures() {
        if(!validTargetFbo()) return;
        mTargetTextureFbo.swapTexture(mGL, mSrcTexture);
    }

    void switchToOpenGL() {
        mContext.switchToOpenGL();
    }

    void switchToSkia() {
        mContext.switchToSkia();
    }

    //! @brief Returns SkCanvas associated with the target texture.
    //! If there is no SkCanvas new SkCanvas is created.
    SkCanvas* requestTargetCanvas() {
        if(mCanvas) {
            mContext.switchToSkia();
        } else {
            mContext.switchToOpenGL();
            requestTargetFbo();

            mContext.switchToSkia();
            GrGLFramebufferInfo fboInfo;
            fboInfo.fFBOID = mTargetTextureFbo.fFBOId;
            fboInfo.fFormat = GR_GL_RGBA8;
            const auto grFbo = GrBackendRenderTarget(mTargetTextureFbo.fWidth,
                                                     mTargetTextureFbo.fHeight,
                                                     0, 8, fboInfo);
            const auto grContext = mContext.requestContext();
            mSurface = SkSurface::MakeFromBackendRenderTarget(
                        grContext, grFbo, kBottomLeft_GrSurfaceOrigin,
                        kRGBA_8888_SkColorType, nullptr, nullptr);
            if(!mSurface) RuntimeThrow("Failed to make SkSurface.");

            mCanvas = mSurface->getCanvas();
        }
        return mCanvas;
    }

    //! @brief Returned texture may be used as one wishes,
    //!  but has to be valid.
    Texture& getSrcTexture() {
        return mSrcTexture;
    }

    //! @brief Returns TextureFrameBuffer associated with the target texture.
    //! If there is no SkCanvas new SkCanvas is created.
    TextureFrameBuffer& requestTargetFbo() {
        if(!mContext.openGLMode())
            RuntimeThrow("Requesting FBO only available in OpenGL mode.\n"
                         "Use SwitchableContext::switchToOpenGL "
                         "prior to this call.");
        if(!validTargetFbo())
            mTargetTextureFbo.gen(mGL, mSrcTexture.fWidth, mSrcTexture.fHeight);
        return mTargetTextureFbo;
    }

    sk_sp<SkImage> requestSrcTextureImageWrapper() {
        const auto grContext = mContext.requestContext();
        if(!grContext) RuntimeThrow("Wrapping texture image with SkImage "
                                    "only available in Skia mode.\n"
                                    "Use SwitchableContext::switchToSkia "
                                    "prior to this call.");
        return SkImage::MakeFromTexture(grContext,
                                        sourceBackedTexture(),
                                        kBottomLeft_GrSurfaceOrigin,
                                        kRGBA_8888_SkColorType,
                                        kPremul_SkAlphaType,
                                        nullptr);
    }

    bool validTargetCanvas() const { return mCanvas; }
    bool validTargetFbo() const { return mTargetTextureFbo.fFBOId != 0; }
private:
    GrBackendTexture sourceBackedTexture() {
        GrGLTextureInfo texInfo;
        texInfo.fID = mSrcTexture.fId;
        texInfo.fFormat = GR_GL_RGBA8;
        texInfo.fTarget = GR_GL_TEXTURE_2D;
        return GrBackendTexture(mSrcTexture.fWidth, mSrcTexture.fHeight,
                                GrMipMapped::kNo, texInfo);
    }

    QGL33* const mGL;
    SwitchableContext& mContext;

    Texture mSrcTexture;
    TextureFrameBuffer mTargetTextureFbo;

    sk_sp<SkSurface> mSurface;
    SkCanvas* mCanvas = nullptr;
};

#endif // GLHELPERS_H
