#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QOpenGLWidget>
#include "glhelpers.h"
#include <string>

#include "skia/skiaincludes.h"

#include <QResizeEvent>
#include <QOpenGLPaintDevice>

class ShaderEffectCreator;
class ShaderEffectProgram;

class GLWindow : public QOpenGLWidget, protected QGL33c {
    Q_OBJECT
public:
    GLWindow(QWidget * const parent = nullptr);
    ~GLWindow();
protected:
    virtual void renderSk(SkCanvas * const canvas,
                          GrContext * const grContext) = 0;
    void resizeGL(int, int) final;
    void initializeGL() final;
    void paintGL() final;
    void showEvent(QShowEvent *e) final;

    void initialize();
    void bindSkia(const int w, const int h);
    void updateFix();

    bool mRebind = false;

    sk_sp<const GrGLInterface> mInterface;
    sk_sp<GrContext> mGrContext;
    sk_sp<SkSurface> mSurface;
    SkCanvas *mCanvas;
};

#endif // GLWINDOW_H
