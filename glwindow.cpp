#include "glwindow.h"
#include "Colors/helpers.h"
#include <QPainter>

static const int kStencilBits = 8;  // Skia needs 8 stencil bits
// If you want multisampling, uncomment the below lines and set a sample count
static const int kMsaaSampleCount = 0; //4;
// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, kMsaaSampleCount);

GLWindow::GLWindow(QScreen *screen)
    : QWindow(screen), QOpenGLFunctions() {
    setSurfaceType(OpenGLSurface);
}
#include <QDebug>
void GLWindow::bindSkia() {
    GrBackendRenderTarget backendRT = GrBackendRenderTarget(
                                        width(), height(),
                                        kMsaaSampleCount, kStencilBits,
                                        kRGBA_half_GrPixelConfig
                                        /*kSkia8888_GrPixelConfig*/, m_fbInfo);

    // setup SkSurface
    // To use distance field text, use commented out SkSurfaceProps instead
    // SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
    //                      SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    //sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGBLinear();
    mSurface = SkSurface::MakeFromBackendRenderTarget(
                                    mGrContext,
                                    backendRT,
                                    kBottomLeft_GrSurfaceOrigin,
                                    nullptr,
                                    //colorSpace,
                                    &props);
    mCanvas = mSurface->getCanvas();
}

void GLWindow::resizeEvent(QResizeEvent *) {
    if(!m_context) return;
    bindSkia();
}

void GLWindow::initialize() {
    glClearColor(1.f, 1.f, 1.f, 1.f);

    //Set blending
    glEnable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // setup GrContext
    mInterface = GrGLCreateNativeInterface();

    // setup contexts
    mGrContext = GrContext::Create(kOpenGL_GrBackend,
                                   (GrBackendContext)mInterface);
    SkASSERT(mGrContext);

    // Wrap the frame buffer object attached to the screen in
    // a Skia render target so Skia can render to it
    //GrGLint buffer;
    //GR_GL_GetIntegerv(mInterface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    m_fbInfo.fFBOID = m_context->defaultFramebufferObject();//buffer;
    bindSkia();

//    qDebug() << "OpenGL Info";
//    qDebug() << "  Vendor: " << reinterpret_cast<const char *>(glGetString(GL_VENDOR));
//    qDebug() << "  Renderer: " << QString((const char*)glGetString(GL_RENDERER));;
//    qDebug() << "  Version: " << reinterpret_cast<const char *>(glGetString(GL_VERSION));
//    qDebug() << "  Shading language: " << reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
//    qDebug() << "  Requested format: " << QSurfaceFormat::defaultFormat();
//    qDebug() << "  Current format:   " << m_context->format();
}

//void glOrthoAndViewportSet(GLuint w, GLuint h) {
//    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(0.0f, w, h, 0.0f, 0.0f, 1.0f);
//    glMatrixMode(GL_MODELVIEW);
//}

#include "Colors/ColorWidgets/colorwidget.h"
void GLWindow::renderNow() {
    if(!isExposed()) return;

    bool needsInitialize = false;
    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(QSurfaceFormat::defaultFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if(needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }


    glOrthoAndViewportSet(width(), height());

    mCanvas->clear(SK_ColorWHITE);
    mCanvas->save();
    renderSk(mCanvas);
//    SkPaint paint;
//    SkPoint gradPoints[2];
//    gradPoints[0] = SkPoint::Make(0.f, 0.f);
//    gradPoints[1] = SkPoint::Make(width(), 0.f);
//    SkColor gradColors[2];
//    gradColors[0] = SkColorSetARGBInline(255, 0, 0, 0);
//    gradColors[1] = SkColorSetARGBInline(0, 125, 125, 125);
//    SkScalar gradPos[2];
//    gradPos[0] = 0.f;
//    gradPos[1] = 1.f;

//    paint.setShader(SkGradientShader::MakeLinear(gradPoints,
//                                                 gradColors,
//                                                 gradPos, 2,
//                                                 SkShader::kClamp_TileMode));
//    paint.setAntiAlias(true);

//    mCanvas->drawRect(SkRect::MakeWH(width(), height()), paint);

    mCanvas->restore();
    mCanvas->flush();


//    if(!m_device) m_device = new QOpenGLPaintDevice;

//    m_device->setSize(size());

//    QPainter painter(m_device);
//    qRender(&painter);

    m_context->swapBuffers(this);

    //m_context->doneCurrent();
}

bool GLWindow::event(QEvent *event) {
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        if(isExposed()) requestUpdate();
        return QWindow::event(event);
    }
}

//void GLWindow::exposeEvent(QExposeEvent *event) {
//    Q_UNUSED(event);

//    if(isExposed()) renderNow();
//}
