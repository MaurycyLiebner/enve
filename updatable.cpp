#include "updatable.h"
#include "mainwindow.h"
#include "updatescheduler.h"

Updatable::Updatable() {

}

void Updatable::beforeUpdate() {
    mBeingProcessed = true;
    mAwaitingUpdate = false;
    mUpdateDependent = mDependent;
    mDependent.clear();
}

void Updatable::afterUpdate() {
    mBeingProcessed = false;
    tellDependentThatFinished();
}

void Updatable::tellDependentThatFinished() {
    foreach(Updatable *dependent, mUpdateDependent) {
        dependent->decDependencies();
    }
    mUpdateDependent.clear();
}

void Updatable::addDependent(Updatable *updatable) {
    if(mAwaitingUpdate) {
        if(mDependent.contains(updatable)) return;
        mDependent << updatable;
        updatable->incDependencies();
    }
}

void Updatable::addScheduler() {
    if(!shouldUpdate() || mAwaitingUpdate) return;
    beforeAddingScheduler();
    mAwaitingUpdate = true;
    MainWindow::getInstance()->addFileCacheUpdateScheduler(
                new AddUpdatableAwaitingUpdateScheduler(this));
}
