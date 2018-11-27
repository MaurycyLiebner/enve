#include "updatable.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "paintcontroler.h"

_ScheduledExecutor::~_ScheduledExecutor() {
    clear();
}

void _ScheduledExecutor::beforeUpdate() {
    _Executor::beforeUpdate();
    mAwaitingUpdate = false;

}

void _ScheduledExecutor::schedulerProccessed() {
    mAwaitingUpdate = true;
    mSchedulerAdded = false;
}

bool _ScheduledExecutor::addScheduler() {
    if(!shouldUpdate() || mSchedulerAdded) return false;

    mFinished = false;
    mSchedulerAdded = true;
    addSchedulerNow();
    return true;
}

void _ScheduledExecutor::addSchedulerNow() {
    MainWindow::getInstance()->addUpdateScheduler(getAsSPtr(this, _ScheduledExecutor));
}

void _ScheduledExecutor::clear() {
    mSchedulerAdded = false;
    mAwaitingUpdate = false;
    _Executor::clear();
}

_Executor::_Executor() {}

void _Executor::setCurrentPaintControler(PaintControler *paintControler) {
    mCurrentPaintControler = paintControler;
}

void _Executor::beforeUpdate() {
    mSelfRef = getAsSPtr(this, _Executor);
    mBeingProcessed = true;
    mCurrentExecutionDependent = mNextExecutionDependent;
    mNextExecutionDependent.clear();
}

void _Executor::updateFinished() {
    mFinished = true;
    mBeingProcessed = false;
    afterUpdate();
    mSelfRef.reset();
}

void _Executor::afterUpdate() {
    mCurrentPaintControler = nullptr;
    tellDependentThatFinished();
}

bool _Executor::isBeingProcessed() { return mBeingProcessed; }

void _Executor::waitTillProcessed() {
    if(mCurrentPaintControler == nullptr) {
        return;
    }
    {
        QEventLoop loop;
        loop.connect(mCurrentPaintControler,
                     SIGNAL(finishedUpdating(int, _Executor*)),
                     SLOT(quit()));
        loop.exec();
    }
}

bool _Executor::readyToBeProcessed() {
    return nDependancies == 0 && !mBeingProcessed;
}

void _Executor::clear() {
    MinimalExecutor::clear();
    mBeingProcessed = false;
    mSelfRef.reset();
    foreach(const MinimalExecutorPtr& dependent, mNextExecutionDependent) {
        if(dependent == nullptr) continue;
        dependent->decDependencies();
    }
    mNextExecutionDependent.clear();
}

void _Executor::addDependent(MinimalExecutor *updatable) {
    if(updatable == nullptr) return;
    if(!finished()) {
        if(listContainsSharedPtr(updatable, mNextExecutionDependent)) return;
        mNextExecutionDependent << updatable;
        updatable->incDependencies();
    }
}

GUI_ThreadExecutor::GUI_ThreadExecutor() {
    mGUIThreadExecution = true;
}

void GUI_ThreadExecutor::addDependent(MinimalExecutor *updatable) {
    if(!finished()) {
        if(listContainsSharedPtr(updatable, mCurrentExecutionDependent)) return;
        mCurrentExecutionDependent << updatable;
        updatable->incDependencies();
    }
}

MinimalExecutor::MinimalExecutor() {}

bool MinimalExecutor::finished() { return mFinished; }

void MinimalExecutor::clear() {
    mFinished = false;
    tellDependentThatFinished();
}

void MinimalExecutor::decDependencies() {
    nDependancies--;
    if(mGUIThreadExecution) {
        if(nDependancies == 0) {
            GUI_process();
            mFinished = true;
            tellDependentThatFinished();
        }
    }
}

void MinimalExecutor::incDependencies() {
    nDependancies++;
}

void MinimalExecutor::GUI_process() {}

void MinimalExecutor::tellDependentThatFinished() {
    foreach(const MinimalExecutorPtr& dependent, mCurrentExecutionDependent) {
        if(dependent == nullptr) continue;
        dependent->decDependencies();
    }
    mCurrentExecutionDependent.clear();
}
