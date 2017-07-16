#include "updatescheduler.h"
#include "mainwindow.h"
#include "Boxes/vectorpath.h"
#include "canvas.h"
#include <QDebug>

QGradientStopsUpdateScheduler::QGradientStopsUpdateScheduler(Gradient *gradient)
{
    mGradient = gradient;
}

void QGradientStopsUpdateScheduler::update()
{
    mGradient->updateQGradientStopsIfNeeded();
}

AddUpdatableAwaitingUpdateScheduler::AddUpdatableAwaitingUpdateScheduler(
        Updatable *updatable) {
    mUpdatable = updatable;
}

void AddUpdatableAwaitingUpdateScheduler::update() {
    mUpdatable->schedulerProccessed();
    MainWindow::getInstance()->getCanvasWindow()->
            addUpdatableAwaitingUpdate(mUpdatable);
}

FileCacheLoadScheduler::FileCacheLoadScheduler(
        FileCacheHandler *cacheHandler) {
    mHandler = cacheHandler;
}

void FileCacheLoadScheduler::update() {
    //mHandler
}
