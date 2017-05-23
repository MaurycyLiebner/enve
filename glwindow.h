#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QWindow>
#include <QOpenGLFunctions>

#undef foreach

// for Release build
//#define GR_GL_CHECK_ERROR false
//#define GR_GL_LOG_CALLS false
#include "GrBackendSurface.h"
#include "GrContext.h"
#include "SDL.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkSurface.h"
#include "SkGradientShader.h"
#include "SkPixelRef.h"

#include "gl/GrGLUtil.h"

#include "gl/GrGLTypes.h"
#include "gl/GrGLFunctions.h"
#include "gl/GrGLInterface.h"

#include <GL/gl.h>
#define foreach Q_FOREACH

#include <QResizeEvent>
#include <QOpenGLPaintDevice>

class GLWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWindow(QScreen *screen = 0);

protected:
    void initialize();
    void renderNow();
    virtual void qRender(QPainter *p) {
        Q_UNUSED(p);
    }

    virtual void render(SkCanvas *canvas) {
        Q_UNUSED(canvas);
    }

    void bindSkia();
    void resizeEvent(QResizeEvent *);

    const GrGLInterface *mInterface;
    GrContext *mGrContext;
    sk_sp<SkSurface> mSurface;
    SkCanvas *mCanvas;

    GrGLFramebufferInfo m_fbInfo;
    QOpenGLContext *m_context = nullptr;
    QOpenGLPaintDevice *m_device = nullptr;

    bool event(QEvent *event);
    //void exposeEvent(QExposeEvent *event);
};

#endif // GLWINDOW_H
