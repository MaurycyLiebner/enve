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

void _ScheduledExecutor::addScheduler() {
    if(!shouldUpdate() || mSchedulerAdded) return;

    mFinished = false;
    mSchedulerAdded = true;
    addSchedulerNow();
}

void _ScheduledExecutor::addSchedulerNow() {
    MainWindow::getInstance()->addUpdateScheduler(this);
}

void _ScheduledExecutor::clear() {
    mSchedulerAdded = false;
    mAwaitingUpdate = false;
    _Executor::clear();
}

void _Executor::waitTillProcessed() {
    if(mCurrentPaintControler == nullptr) {
        return;
    }
    {
        QEventLoop loop;
        loop.connect(mCurrentPaintControler,
                     SIGNAL(finishedUpdating(int,_Executor*)),
                     SLOT(quit()));
        loop.exec();
    }
}
