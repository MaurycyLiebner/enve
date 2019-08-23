#include "gpupostprocessor.h"
#include <QImage>
#include <QOpenGLTexture>
#include "skia/skqtconversions.h"
#include "Tasks/taskscheduler.h"

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
    mAllDone = true;
    for(const auto& task : _mHandledProcesses) {
        const bool nextStep = !task->waitingToCancel() && task->nextStep();
        if(nextStep) TaskScheduler::sGetInstance()->queCpuTaskFastTrack(task);
        else task->finishedProcessing();
    }
    _mHandledProcesses.clear();
    handleScheduledProcesses();
    TaskScheduler::sGetInstance()->afterCpuGpuTaskFinished();
}
