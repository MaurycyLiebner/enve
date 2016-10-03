#include "updatescheduler.h"
#include "mainwindow.h"

MappedPathUpdateScheduler::MappedPathUpdateScheduler(VectorPath *path) : UpdateScheduler() {
    mPath = path;
}

void MappedPathUpdateScheduler::update() {
    mPath->updateMappedPathIfNeeded();
}

PathUpdateScheduler::PathUpdateScheduler(VectorPath *path) : UpdateScheduler() {
    mPath = path;
}

void PathUpdateScheduler::update() {
    mPath->updatePathIfNeeded();
}

OutlineUpdateScheduler::OutlineUpdateScheduler(VectorPath *path)
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
