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
