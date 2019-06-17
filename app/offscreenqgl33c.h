#ifndef OFFSCREENQGL33C_H
#define OFFSCREENQGL33C_H

#include "glhelpers.h"
#include "exceptions.h"
#include <QOffscreenSurface>

class OffscreenQGL33c : protected QGL33c {
public:
    OffscreenQGL33c();

    void initialize() {
        mOffscreenSurface = new QOffscreenSurface(nullptr, nullptr);
        mOffscreenSurface->create();
        mContext = new QOpenGLContext();
        mContext->setShareContext(QOpenGLContext::globalShareContext());
        if(!mContext->create())
            RuntimeThrow("Creating GL context failed.");
    }

    void makeCurrent() {
        if(!mContext->makeCurrent(mOffscreenSurface))
            RuntimeThrow("Making GL context current failed.");
        if(!mInitialized) {
            if(!initializeOpenGLFunctions())
                RuntimeThrow("Initializing GL functions failed.");
            mInitialized = true;
        }
    }

    void doneCurrent() {
        mContext->doneCurrent();
    }

    void moveContextToThread(QThread * const thread) {
        mContext->moveToThread(thread);
    }
private:
    bool mInitialized = false;
    QOpenGLContext* mContext = nullptr;
    QOffscreenSurface *mOffscreenSurface = nullptr;
};

#endif // OFFSCREENQGL33C_H
