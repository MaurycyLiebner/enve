#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QWindow>
#include "glhelpers.h"
#include <string>

#include "skiaincludes.h"

#include <QResizeEvent>
#include <QOpenGLPaintDevice>

class GLWindow : public QWindow, protected QGL33c {
    Q_OBJECT
public:
    GLWindow(QScreen *screen = nullptr);
    ~GLWindow();

    void makeContextCurrent_TEST() {
        Q_ASSERT(mContext->makeCurrent(this));
    }

    void contextDoneCurrent_TEST() {
        mContext->doneCurrent();
    }
protected:
    void initialize();
    void renderNow();
    virtual void qRender(QPainter *p) {
        Q_UNUSED(p);
    }

    virtual void renderSk(SkCanvas * const canvas,
                          GrContext * const grContext) = 0;

    void bindSkia();
    void resizeEvent(QResizeEvent *);

    GLuint mPlainSquareVAO;
    GLuint mTexturedSquareVAO;

    sk_sp<const GrGLInterface> mInterface;
    sk_sp<GrContext> mGrContext;
    sk_sp<SkSurface> mSurface;
    SkCanvas *mCanvas;

    GrGLFramebufferInfo mFbInfo;
    QOpenGLContext *mContext = nullptr;
    //QOpenGLPaintDevice *mDevice = nullptr;

    bool event(QEvent *event);
    //void exposeEvent(QExposeEvent *event);
private:
    void checkCompileErrors(GLuint shader, std::string type);
    void iniBlurProgram();
};

#endif // GLWINDOW_H
