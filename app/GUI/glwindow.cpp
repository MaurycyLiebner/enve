#include "glwindow.h"
#include "colorhelpers.h"
#include <QPainter>
#include <QDebug>
#include "exceptions.h"

GLWindow::GLWindow(QScreen *screen)
    : QWindow(screen) {
    setSurfaceType(OpenGLSurface);
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

void GLWindow::bindSkia() {
    GrBackendRenderTarget backendRT = GrBackendRenderTarget(
                                        width(), height(),
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

void GLWindow::resizeEvent(QResizeEvent *) {
    if(!mContext) return;
    try {
        bindSkia();
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

#include "glhelpers.h"
#include "ColorWidgets/colorwidgetshaders.h"
#include "GPUEffects/gpurastereffect.h"
void GLWindow::iniRasterEffectPrograms() {
    QDirIterator dirIt(QDir::homePath() + "/.AniVect/GPURasterEffects", QDirIterator::NoIteratorFlags);
    while(dirIt.hasNext()) {
        const QString path = dirIt.next();
        const QFileInfo fileInfo(path);
        if(!fileInfo.isFile()) continue;
        if(fileInfo.suffix() != "gre") continue;
        try {
            GPURasterEffectCreator::sLoadFromFile(this, path);
        } catch(...) {
            QString errMsg = "Error while loading GPURasterEffect from '" + path + "'.";
            RuntimeThrow(errMsg.toStdString());
        }
    }
}

void GLWindow::initialize() {
    glClearColor(1, 1, 1, 1);

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
    mFbInfo.fFBOID = mContext->defaultFramebufferObject();//buffer;
    mFbInfo.fFormat = GR_GL_RGBA8;//buffer;

    try {
        bindSkia();
    } catch(...) {
        RuntimeThrow("Failed to bind SKIA.");
    }

    try {
        iniPlainVShaderVBO(this);
        iniPlainVShaderVAO(this, mPlainSquareVAO);
        iniTexturedVShaderVBO(this);
        iniTexturedVShaderVAO(this, mTexturedSquareVAO);
        iniColorPrograms(this);
    } catch(...) {
        RuntimeThrow("Error initializing programs.");
    }

    try {
        iniRasterEffectPrograms();
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
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

void GLWindow::renderNow() {
    if(!isExposed()) return;

    try {
        bool needsInitialize = false;
        if(!mContext) {
            mContext = new QOpenGLContext(this);
    //        mContext->setFormat(QSurfaceFormat::defaultFormat());
            mContext->setShareContext(QOpenGLContext::globalShareContext());
            if(!mContext->create()) {
                RuntimeThrow("Creating GL context failed.");
            }

            needsInitialize = true;
        }
        if(!mContext->makeCurrent(this)) {
            RuntimeThrow("Making GL context current failed.");
        }

        if(needsInitialize) {
            if(!initializeOpenGLFunctions()) {
                RuntimeThrow("Initializing GL functions failed.");
            }
            initialize();
        }
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mContext->defaultFramebufferObject());
    glViewport(0, 0, width(), height());

    renderSk(mCanvas, mGrContext.get());
    mCanvas->flush();

    mContext->swapBuffers(this);
    mContext->doneCurrent();
}

bool GLWindow::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        //QWindow::event(event);
        return true;
    case QEvent::WindowStateChange:
    case QEvent::WindowActivate:
    case QEvent::Expose:
    case QEvent::Resize:
        requestUpdate();
        [[fallthrough]];
       // return true;
    default:
//        qDebug() << event->type();
//        if(isExposed()) {
//            QEvent::Type type = event->type();
//            if(type == QEvent::MouseMove ||
//                type == QEvent::MouseButtonPress ||
//                type == QEvent::) {
//                requestUpdate();
//            }
//        }
        return QWindow::event(event);
    }
}

//void GLWindow::exposeEvent(QExposeEvent *event) {
//    Q_UNUSED(event);

//    if(isExposed()) renderNow();
//}
