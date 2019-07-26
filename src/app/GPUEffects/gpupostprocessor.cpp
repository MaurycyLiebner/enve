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
    for(const auto& task : _mHandledProcesses) {
        if(task->nextStep()) {
            TaskScheduler::sGetInstance()->scheduleCPUTask(task);
        } else task->finishedProcessing();
    }
    _mHandledProcesses.clear();
    handleScheduledProcesses();
    if(mFinished) emit processedAll();
}
