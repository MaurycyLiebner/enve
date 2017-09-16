#include "updatable.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "paintcontroler.h"

Updatable::Updatable() {
}

Updatable::~Updatable() {
}

void Updatable::beforeUpdate() {
    Executable::beforeUpdate();
    mAwaitingUpdate = false;
    mUpdateDependent = mDependent;
    mDependent.clear();
}

void Updatable::afterUpdate() {
    tellDependentThatFinished();
}

void Updatable::schedulerProccessed() {
    mAwaitingUpdate = true;
    mSchedulerAdded = false;
}

void Updatable::tellDependentThatFinished() {
    foreach(Updatable *dependent, mUpdateDependent) {
        dependent->decDependencies();
    }
    mUpdateDependent.clear();
}

void Updatable::addDependent(Updatable *updatable) {
    //if(mAwaitingUpdate) {
    if(mDependent.contains(updatable)) return;
    mDependent << updatable;
    updatable->incDependencies();
    //}
}

void Updatable::addScheduler() {
    if(!shouldUpdate() || mSchedulerAdded) return;

    mFinished = false;
    mSchedulerAdded = true;
    addSchedulerNow();
}

void Updatable::addSchedulerNow() {
    MainWindow::getInstance()->addUpdateScheduler(this);
}

void Updatable::clear() {
    mDependent.clear();
    mUpdateDependent.clear();   
    mSchedulerAdded = false;
    mAwaitingUpdate = false;
    Executable::clear();
}

void Executable::waitTillProcessed() {
    if(mCurrentPaintControler == NULL) return;
    {
        QEventLoop loop;
        loop.connect(mCurrentPaintControler,
                     SIGNAL(finishedUpdating(int,Executable*)),
                     SLOT(quit()));
        loop.exec();
    }
}
