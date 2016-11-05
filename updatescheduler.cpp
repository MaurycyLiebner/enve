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

AwaitUpdateUpdateScheduler::AwaitUpdateUpdateScheduler(BoundingBox *target)
{
    mTarget = target;
}

void AwaitUpdateUpdateScheduler::update()
{
    mTarget->setAwaitUpdateScheduled(false);
    mTarget->awaitUpdate();
}
