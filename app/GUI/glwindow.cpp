#include "glwindow.h"
#include "colorhelpers.h"
#include <QPainter>
#include <QDebug>
#include "exceptions.h"

GLWindow::GLWindow(QWidget * const parent)
    : QOpenGLWidget(parent) {
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}

GLWindow::~GLWindow() {
//    mGrContext->abandonContext();
//    delete mGrContext;
//    delete mInterface;
//    mContext->makeCurrent(this);
//    mSurface.reset();
//    delete mGrContext;
//    mContext->doneCurrent();
}

void GLWindow::bindSkia(const int w, const int h) {
    GrBackendRenderTarget backendRT = GrBackendRenderTarget(
                                        w, h,
                                        0, 8, // (optional) 4, 8,
                                        mFbInfo
                                        /*kRGBA_half_GrPixelConfig*/
                                        /*kSkia8888_GrPixelConfig*/);

    // setup SkSurface
    // To use distance field text, use commented out SkSurfaceProps instead
    // SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
    //                      SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

//    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
    mSurface = SkSurface::MakeFromBackendRenderTarget(
                                    mGrContext.get(),
                                    backendRT,
                                    kBottomLeft_GrSurfaceOrigin,
                                    SkColorType::kRGBA_8888_SkColorType,
                                    nullptr/*colorSpace*/,
                                    &props);
    if(!mSurface) RuntimeThrow("Failed to wrap buffer into SkSurface.");
    mCanvas = mSurface->getCanvas();
}

void GLWindow::resizeGL(int w, int h) {
    try {
        bindSkia(w, h);
        update();
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

void GLWindow::initializeGL() {
    try {
        if(context()->shareContext() != QOpenGLContext::globalShareContext()) {
            context()->setShareContext(QOpenGLContext::globalShareContext());
        }

        if(!initializeOpenGLFunctions()) {
            RuntimeThrow("Initializing GL functions failed.");
        }
        initialize();
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
    }
}

void GLWindow::initialize() {
    glClearColor(0, 0, 0, 1);

    //Set blending
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // setup GrContext
    mInterface = GrGLMakeNativeInterface();
    if(!mInterface) {
        RuntimeThrow("Failed to make native interface.");
    }
//    GrContextOptions options;
    // setup contexts
    mGrContext = GrContext::MakeGL(mInterface/*, options*/);
    if(!mGrContext) {
        RuntimeThrow("Failed to make GrContext.");
    }
    // Wrap the frame buffer object attached to the screen in
    // a Skia render target so Skia can render to it
    //GrGLint buffer;
    //GR_GL_GetIntegerv(mInterface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    mFbInfo.fFBOID = context()->defaultFramebufferObject();//buffer;
    mFbInfo.fFormat = GR_GL_RGBA8;//buffer;

    try {
        bindSkia(width(), height());
    } catch(...) {
        RuntimeThrow("Failed to bind SKIA.");
    }

//    qDebug() << "OpenGL Info";
//    qDebug() << "  Vendor: " << rcConstChar(glGetString(GL_VENDOR));
//    qDebug() << "  Renderer: " << QString((const char*)glGetString(GL_RENDERER));;
//    qDebug() << "  Version: " << rcConstChar(glGetString(GL_VERSION));
//    qDebug() << "  Shading language: " << rcConstChar(glGetString(GL_SHADING_LANGUAGE_VERSION));
//    qDebug() << "  Requested format: " << QSurfaceFormat::defaultFormat();
//    qDebug() << "  Current format:   " << mContext->format();
}

//void glOrthoAndViewportSet(GLuint w, GLuint h) {
//    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(0.0f, w, h, 0.0f, 0.0f, 1.0f);
//    glMatrixMode(GL_MODELVIEW);
//}



#include "GPUEffects/gpupostprocessor.h"
#include "taskscheduler.h"
void GLWindow::paintGL() {
    //glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());
    glViewport(0, 0, width(), height());

    renderSk(mCanvas, mGrContext.get());
    mCanvas->flush();
}
