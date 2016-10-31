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
