#include "updatescheduler.h"
#include "mainwindow.h"

MappedPathUpdateScheduler::MappedPathUpdateScheduler(PathBox *path) : UpdateScheduler() {
    mPath = path;
}

void MappedPathUpdateScheduler::update() {
    mPath->updateMappedPathIfNeeded();
}

PathUpdateScheduler::PathUpdateScheduler(PathBox *path) : UpdateScheduler() {
    mPath = path;
}

void PathUpdateScheduler::update() {
    mPath->updatePathIfNeeded();
}

OutlineUpdateScheduler::OutlineUpdateScheduler(PathBox *path)
{
    mPath = path;
}

void OutlineUpdateScheduler::update()
{
    mPath->updateOutlinePathIfNeeded();
}

QGradientStopsUpdateScheduler::QGradientStopsUpdateScheduler(Gradient *gradient)
{
    mGradient = gradient;
}

void QGradientStopsUpdateScheduler::update()
{
    mGradient->updateQGradientStopsIfNeeded();
}
