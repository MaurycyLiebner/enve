#include "gpupostprocessor.h"
#include <QImage>
#include <QOpenGLTexture>
#include "skia/skqtconversions.h"
#include "taskscheduler.h"

GpuPostProcessor::GpuPostProcessor() {
    connect(this, &QThread::finished,
            this, &GpuPostProcessor::afterProcessed);
}

void GpuPostProcessor::initialize() {
    OffscreenQGL33c::initialize();
    moveContextToThread(this);
}

void GpuPostProcessor::afterProcessed() {
    if(unhandledException())
        gPrintExceptionCritical(handleException());
    mFinished = true;
    for(const auto& process : _mHandledProcesses) {
        if(process->nextStep()) {
            TaskScheduler::sGetInstance()->queCPUTask(process);
        } else process->finishedProcessing();
    }
    _mHandledProcesses.clear();
    handleScheduledProcesses();
    if(mFinished) emit processedAll();
}
