#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QWindow>
#include <QOpenGLFunctions>

#include "skiaincludes.h"

#include <QResizeEvent>
#include <QOpenGLPaintDevice>

class GLWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWindow(QScreen *screen = 0);
    ~GLWindow();
protected:
    void initialize();
    void renderNow();
    virtual void qRender(QPainter *p) {
        Q_UNUSED(p);
    }

    virtual void renderSk(SkCanvas *canvas) {
        Q_UNUSED(canvas);
    }

    void bindSkia();
    void resizeEvent(QResizeEvent *);

    sk_sp<const GrGLInterface> mInterface;
    sk_sp<GrContext> mGrContext;
    sk_sp<SkSurface> mSurface;
    SkCanvas *mCanvas;

    GrGLFramebufferInfo mFbInfo;
    QOpenGLContext *m_context = nullptr;
    //QOpenGLPaintDevice *m_device = nullptr;

    bool event(QEvent *event);
    //void exposeEvent(QExposeEvent *event);
};

#endif // GLWINDOW_H
