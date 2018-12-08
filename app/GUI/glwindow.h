#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QWindow>
#include <QOpenGLFunctions_3_3_Core>
#include <string>

#include "skiaincludes.h"

#include <QResizeEvent>
#include <QOpenGLPaintDevice>

class GLWindow : public QWindow, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    GLWindow(QScreen *screen = nullptr);
    ~GLWindow();
protected:
    void iniVertData();
    void initialize();
    void renderNow();
    virtual void qRender(QPainter *p) {
        Q_UNUSED(p);
    }

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
    //QOpenGLPaintDevice *mDevice = nullptr;

    bool event(QEvent *event);
    //void exposeEvent(QExposeEvent *event);
private:
    void checkCompileErrors(GLuint shader, std::string type);
    void iniProgram(GLuint &program,
                    const std::string &vShaderPath,
                    const std::string &fShaderPath);
    void iniBlurProgram();
};

#endif // GLWINDOW_H
