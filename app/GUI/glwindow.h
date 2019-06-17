#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QWindow>
#include "glhelpers.h"
#include <string>

#include "skia/skiaincludes.h"

#include <QResizeEvent>
#include <QOpenGLPaintDevice>

class GPURasterEffectCreator;
class GPURasterEffectProgram;

class GLWindow : public QWindow, protected QGL33c {
    Q_OBJECT
public:
    GLWindow(QScreen *screen = nullptr);
    ~GLWindow();
protected:
    void initialize();
    void renderNow();

    virtual void renderSk(SkCanvas * const canvas,
                          GrContext * const grContext) = 0;

    void bindSkia();
    void resizeEvent(QResizeEvent *);

    sk_sp<const GrGLInterface> mInterface;
    sk_sp<GrContext> mGrContext;
    sk_sp<SkSurface> mSurface;
    SkCanvas *mCanvas;

    GrGLFramebufferInfo mFbInfo;
    QOpenGLContext *mContext = nullptr;

    bool event(QEvent *event);
    //void exposeEvent(QExposeEvent *event);
};

#endif // GLWINDOW_H
