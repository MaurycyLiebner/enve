#include "updatescheduler.h"
#include "mainwindow.h"

QGradientStopsUpdateScheduler::QGradientStopsUpdateScheduler(Gradient *gradient)
{
    mGradient = gradient;
}

void QGradientStopsUpdateScheduler::update()
{
    mGradient->updateQGradientStopsIfNeeded();
}

AddBoxAwaitingUpdateScheduler::AddBoxAwaitingUpdateScheduler(Canvas *canvas) {
    mCanvas = canvas;
}

void AddBoxAwaitingUpdateScheduler::update() {
    MainWindow::getInstance()->getCanvasWidget()->addBoxAwaitingUpdate(mCanvas);
}

PathUpdateScheduler::PathUpdateScheduler(PathBox *path) {
    mPath = path;
}

void PathUpdateScheduler::update() {
    mPath->updatePathIfNeeded();
    mPath->updateOutlinePathIfNeeded();
}
