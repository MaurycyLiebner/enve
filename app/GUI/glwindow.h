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

    GLuint mPlainSquareVAO;
    GLuint mTexturedSquareVAO;

    sk_sp<const GrGLInterface> mInterface;
    sk_sp<GrContext> mGrContext;
    sk_sp<SkSurface> mSurface;
    SkCanvas *mCanvas;

    GrGLFramebufferInfo mFbInfo;
    QOpenGLContext *mContext = nullptr;

    bool event(QEvent *event);
    //void exposeEvent(QExposeEvent *event);
signals:
    void programChanged(GPURasterEffectProgram*);
    void queAfterProgramsChanged();
private:
    struct NewGPUEffect {
        QString fGrePath;
        QString fFragPath;

        bool operator==(const NewGPUEffect& other) const {
            return fGrePath == other.fGrePath;
        }
    };

    struct FragReload {
        GPURasterEffectCreator *fCreator;
        QString fFragPath;

        bool operator==(const FragReload& other) const {
            return fCreator == other.fCreator;
        }
    };

    bool mWaitingFragReloads = false;
    QList<FragReload> mFragReloads;

    bool mWaitingGPUEffects = false;
    QList<NewGPUEffect> mNewGPUEffects;
    QList<QString> mLoadedGREPaths;

    void checkCompileErrors(GLuint shader, std::string type);
    void iniRasterEffectProgram(const QString& path);
    void iniRasterEffectPrograms();
    void iniCustomPathEffects();
};

#endif // GLWINDOW_H
