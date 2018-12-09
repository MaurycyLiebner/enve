#include "glwindow.h"
#include "GUI/ColorWidgets/helpers.h"
#include <QPainter>

static const int kStencilBits = 8;  // Skia needs 8 stencil bits
// If you want multisampling, uncomment the below lines and set a sample count
static const int kMsaaSampleCount = 0; //4;
// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, kMsaaSampleCount);

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

#include <QDebug>
void GLWindow::bindSkia() {
    GrBackendRenderTarget backendRT = GrBackendRenderTarget(
                                        width(), height(),
                                        kMsaaSampleCount, kStencilBits,
                                        mFbInfo
                                        /*kRGBA_half_GrPixelConfig*/
                                        /*kSkia8888_GrPixelConfig*/);

    // setup SkSurface
    // To use distance field text, use commented out SkSurfaceProps instead
    // SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
    //                      SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGB();
    mSurface = SkSurface::MakeFromBackendRenderTarget(
                                    mGrContext.get(),
                                    backendRT,
                                    kBottomLeft_GrSurfaceOrigin,
                                    SkColorType::kRGBA_8888_SkColorType,
                                    nullptr/*colorSpace*/,
                                    &props);
    mCanvas = mSurface->getCanvas();
}

void GLWindow::resizeEvent(QResizeEvent *) {
    if(!mContext) return;
    bindSkia();
}
#include "glhelpers.h"
#include "ColorWidgets/colorwidgetshaders.h"

void GLWindow::iniBlurProgram() {
    iniProgram(this, GL_BLUR_PROGRAM,
               GL_TEXTURED_VERT,
               "/home/ailuropoda/Dev/AniVect/src/shaders/blur.frag");
    glUseProgram(GL_BLUR_PROGRAM);

    GLint texLocation = glGetUniformLocation(
                GL_BLUR_PROGRAM, "texture");
    glUniform1i(texLocation, 0);
}

void GLWindow::initialize() {
    glClearColor(1.f, 1.f, 1.f, 1.f);

    //Set blending
    glEnable(GL_BLEND );
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // setup GrContext
    mInterface = GrGLMakeNativeInterface();

//    GrContextOptions options;
    // setup contexts
    mGrContext = GrContext::MakeGL(mInterface/*, options*/);
    SkASSERT(mGrContext);

    // Wrap the frame buffer object attached to the screen in
    // a Skia render target so Skia can render to it
    //GrGLint buffer;
    //GR_GL_GetIntegerv(mInterface, GR_GL_FRAMEBUFFER_BINDING, &buffer);
    mFbInfo.fFBOID = mContext->defaultFramebufferObject();//buffer;
    mFbInfo.fFormat = GR_GL_RGBA8;//buffer;

    bindSkia();

    iniTexturedVShaderData(this);
    iniBlurProgram();

//    qDebug() << "OpenGL Info";
//    qDebug() << "  Vendor: " << reinterpret_cast<const char *>(glGetString(GL_VENDOR));
//    qDebug() << "  Renderer: " << QString((const char*)glGetString(GL_RENDERER));;
//    qDebug() << "  Version: " << reinterpret_cast<const char *>(glGetString(GL_VERSION));
//    qDebug() << "  Shading language: " << reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
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

void draw(SkCanvas* canvas) {
    SkBlendMode modes[] = {
        SkBlendMode::kClear,
        SkBlendMode::kSrc,
        SkBlendMode::kDst,
        SkBlendMode::kSrcOver,
        SkBlendMode::kDstOver,
        SkBlendMode::kSrcIn,
        SkBlendMode::kDstIn,
        SkBlendMode::kSrcOut,
        SkBlendMode::kDstOut,
        SkBlendMode::kSrcATop,
        SkBlendMode::kDstATop,
        SkBlendMode::kXor,
        SkBlendMode::kPlus,
        SkBlendMode::kModulate,
        SkBlendMode::kScreen,
        SkBlendMode::kOverlay,
        SkBlendMode::kDarken,
        SkBlendMode::kLighten,
        SkBlendMode::kColorDodge,
        SkBlendMode::kColorBurn,
        SkBlendMode::kHardLight,
        SkBlendMode::kSoftLight,
        SkBlendMode::kDifference,
        SkBlendMode::kExclusion,
        SkBlendMode::kMultiply,
        SkBlendMode::kHue,
        SkBlendMode::kSaturation,
        SkBlendMode::kColor,
        SkBlendMode::kLuminosity,
    };
    SkRect rect = SkRect::MakeWH(64.0f, 64.0f);
    SkPaint text, stroke, src, dst;
    stroke.setStyle(SkPaint::kStroke_Style);
    text.setTextSize(24.0f);
    text.setAntiAlias(true);
    SkPoint srcPoints[2] = {
        SkPoint::Make(0.0f, 0.0f),
        SkPoint::Make(64.0f, 0.0f)
    };
    SkColor srcColors[2] = {
        SK_ColorMAGENTA & 0x00FFFFFF,
        SK_ColorMAGENTA};
    src.setShader(SkGradientShader::MakeLinear(
                srcPoints, srcColors, nullptr, 2,
                SkShader::kClamp_TileMode, 0, nullptr));

    SkPoint dstPoints[2] = {
        SkPoint::Make(0.0f, 0.0f),
        SkPoint::Make(0.0f, 64.0f)
    };
    SkColor dstColors[2] = {
        SK_ColorCYAN & 0x00FFFFFF,
        SK_ColorCYAN};
    dst.setShader(SkGradientShader::MakeLinear(
                dstPoints, dstColors, nullptr, 2,
                SkShader::kClamp_TileMode, 0, nullptr));
    canvas->clear(SK_ColorWHITE);
    size_t N = sizeof(modes) / sizeof(modes[0]);
    size_t K = (N - 1) / 3 + 1;
    SkASSERT(K * 64 == 640);  // tall enough
    for(size_t i = 0; i < N; ++i) {
        SkAutoCanvasRestore autoCanvasRestore(canvas, true);
        canvas->translate(192.0f * (i / K), 64.0f * (i % K));
        const char* desc = SkBlendMode_Name(modes[i]);
        canvas->drawText(desc, strlen(desc), 68.0f, 30.0f, text);
        canvas->clipRect(SkRect::MakeWH(64.0f, 64.0f));
        canvas->drawColor(SK_ColorLTGRAY);
        (void)canvas->saveLayer(nullptr, nullptr);
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawPaint(dst);
        src.setBlendMode(modes[i]);
        canvas->drawPaint(src);
        canvas->drawRect(rect, stroke);
    }
}

void GLWindow::renderNow() {
    if(!isExposed()) return;

    bool needsInitialize = false;
    if(!mContext) {
        mContext = new QOpenGLContext(this);
//        mContext->setFormat(QSurfaceFormat::defaultFormat());
        //mContext->setShareContext(QOpenGLContext::globalShareContext());
        Q_ASSERT(mContext->create());

        needsInitialize = true;
    }

    Q_ASSERT(mContext->makeCurrent(this));

    if(needsInitialize) {
        Q_ASSERT(initializeOpenGLFunctions());
        initialize();
    }
    return;

    glBindFramebuffer(GL_FRAMEBUFFER, mContext->defaultFramebufferObject());

    glOrthoAndViewportSet(width(), height());

    //mCanvas->save();
    //draw(mCanvas);
    renderSk(mCanvas, mGrContext.get());
//    SkPaint paint;
//    SkPoint gradPoints[2];
//    gradPoints[0] = SkPoint::Make(0.f, 0.f);
//    gradPoints[1] = SkPoint::Make(width(), 0.f);
//    SkColor gradColors[2];
//    gradColors[0] = SkColorSetARGB(255, 0, 0, 0);
//    gradColors[1] = SkColorSetARGB(0, 125, 125, 125);
//    SkScalar gradPos[2];
//    gradPos[0] = 0.f;
//    gradPos[1] = 1.f;

//    paint.setShader(SkGradientShader::MakeLinear(gradPoints,
//                                                 gradColors,
//                                                 gradPos, 2,
//                                                 SkShader::kClamp_TileMode));
//    paint.setAntiAlias(true);

//    mCanvas->drawRect(SkRect::MakeWH(width(), height()), paint);

    //mCanvas->restore();
    mCanvas->flush();


//    if(!mDevice) mDevice = new QOpenGLPaintDevice;

//    mDevice->setSize(size());

//    QPainter painter(mDevice);
//    qRender(&painter);

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
