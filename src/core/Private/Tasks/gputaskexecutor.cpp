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

#include "gputaskexecutor.h"

QAtomicList<stdsptr<eTask>> GpuTaskExecutor::sTasks;
QAtomicInt GpuTaskExecutor::sUseCount;

GpuTaskExecutor::GpuTaskExecutor() :
    TaskExecutor(sUseCount, sTasks) {}

void GpuTaskExecutor::sAddTask(const stdsptr<eTask>& ready) {
    sTasks.appendAndNotifyAll(ready);
}

void GpuTaskExecutor::sAddTasks(const QList<stdsptr<eTask>>& ready) {
    sTasks.appendAndNotifyAll(ready);
}

int GpuTaskExecutor::sUsageCount() {
    return sUseCount;
}

int GpuTaskExecutor::sWaitingTasks() {
    return sTasks.count();
}

bool GpuTaskExecutor::unhandledException() const {
    return static_cast<bool>(mProcessException);
}

std::exception_ptr GpuTaskExecutor::handleException() {
    std::exception_ptr exc;
    mProcessException.swap(exc);
    return exc;
}

void GpuTaskExecutor::initialize(QThread* const thread) {
    OffscreenQGL33c::initialize();
    moveContextToThread(thread);
}

void GpuTaskExecutor::initializeContext() {
    mInterface = GrGLMakeNativeInterface();
    if(!mInterface) RuntimeThrow("Failed to make native interface.");
    const auto grContext = GrContext::MakeGL(mInterface);
    if(!grContext) RuntimeThrow("Failed to make GrContext.");
    GLuint textureSquareVAO;
    iniTexturedVShaderVAO(this, textureSquareVAO);
    mContext.setContext(grContext, textureSquareVAO);
    glClearColor(0, 0, 0, 0);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    const GLenum glError = glGetError();
    if(glError != GL_NO_ERROR)
        RuntimeThrow("OpenGL error " + std::to_string(glError));
}

void GpuTaskExecutor::processTask(eTask& task) {
    task.processGpu(this, mContext);
}

void GpuTaskExecutor::start() {
    makeCurrent();
    if(!mInitialized) {
        try {
            initializeContext();
        } catch(...) {
            setException(std::current_exception());
            doneCurrent();
            QThread::currentThread()->quit();
            return;
        }
        mInitialized = true;
    }
    processLoop();
    doneCurrent();
}

void GpuTaskExecutor::setException(const std::exception_ptr& exception) {
    mProcessException = exception;
}
