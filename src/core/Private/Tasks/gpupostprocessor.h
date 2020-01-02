// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef GPUPOSTPROCESSOR_H
#define GPUPOSTPROCESSOR_H
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include "offscreenqgl33c.h"
#include "smartPointers/stdselfref.h"
#include "skia/skiaincludes.h"
#include "glhelpers.h"
#include "RasterEffects/rastereffect.h"
#include "Tasks/updatable.h"

#include <QOpenGLFramebufferObject>
#include "exceptions.h"
class GpuPostProcessor : public QThread, protected OffscreenQGL33c {
    Q_OBJECT
public:
    GpuPostProcessor();
    void initialize();

    //! @brief Adds a new task and starts processing it if is not busy.
    void addToProcess(const stdsptr<eTask>& scheduled) {
        //scheduled->afterProcessed(); return;
        Q_ASSERT(scheduled->hardwareSupport() != HardwareSupport::cpuOnly);
        mScheduledProcesses << scheduled;
        handleScheduledProcesses();
    }

    void clear() {
        mScheduledProcesses.clear();
    }

    //! @brief Starts processing scheduled tasks if is not busy.
    void handleScheduledProcesses() {
        if(mScheduledProcesses.isEmpty()) return;
        if(!mAllDone) return;
        mAllDone = false;
        _mHandledProcesses << mScheduledProcesses.takeFirst();
        start();
    }

    //! @brief Returns true if nothing is waiting/being processed.
    bool allDone() const { return mAllDone; }
private:
    void afterProcessed();
protected:
    void run() override {
        try {
            processTasks();
        } catch(...) {
            setException(std::current_exception());
        }
    }

    void setException(const std::exception_ptr& exception) {
        mProcessException = exception;
    }

    void processTasks() {
        if(_mHandledProcesses.isEmpty()) return;
        makeCurrent();
        if(!mInitialized) {
            mInterface = GrGLMakeNativeInterface();
            if(!mInterface) RuntimeThrow("Failed to make native interface.");
            const auto grContext = GrContext::MakeGL(mInterface);
            if(!grContext) RuntimeThrow("Failed to make GrContext.");
            GLuint textureSquareVAO;
            iniTexturedVShaderVAO(this, textureSquareVAO);
            mInitialized = true;
            mContext.setContext(grContext, textureSquareVAO);

            glClearColor(0, 0, 0, 0);
            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }

        for(const auto& scheduled : _mHandledProcesses) {
            try {
                scheduled->processGpu(this, mContext);
            } catch(...) {
                scheduled->setException(std::current_exception());
            }
        }
        doneCurrent();
        //mFrameBuffer->bindDefault();
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool unhandledException() const {
        return static_cast<bool>(mProcessException);
    }

    std::exception_ptr handleException() {
        std::exception_ptr exc;
        mProcessException.swap(exc);
        return exc;
    }

    sk_sp<const GrGLInterface> mInterface;
    SwitchableContext mContext;
    std::exception_ptr mProcessException;
    bool mAllDone = true;
    bool mInitialized = false;
    GLuint _mTextureSquareVAO;
    QList<stdsptr<eTask>> _mHandledProcesses;
    QList<stdsptr<eTask>> mScheduledProcesses;
    //QOpenGLFramebufferObject* mFrameBuffer = nullptr;
};

#endif // GPUPOSTPROCESSOR_H
