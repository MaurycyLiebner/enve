#include "updatable.h"
#include "mainwindow.h"
#include "updatescheduler.h"
#include "global.h"

Updatable::Updatable() {
    mId = gUpdatableId;
    gUpdatableId++;
}

Updatable::~Updatable() {
    qDebug() << "deleting: " << mId;
}

void Updatable::beforeUpdate() {
    qDebug() << "before update: " << mId;
    mBeingProcessed = true;
    mAwaitingUpdate = false;
    mUpdateDependent = mDependent;
    mDependent.clear();
}

void Updatable::afterUpdate() {
    qDebug() << "after update: " << mId;
    mFinished = true;
    mBeingProcessed = false;
    tellDependentThatFinished();
}

void Updatable::schedulerProccessed() {
    qDebug() << "scheduler proessed: " << mId;
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
    qDebug() << "add scheduler: " << mId;

    mSchedulerAdded = true;
    addSchedulerNow();
}

void Updatable::addSchedulerNow() {
    MainWindow::getInstance()->addUpdateScheduler(this);
}
